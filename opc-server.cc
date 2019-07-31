/**
 *  OPC image packet receiver.
 */
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <string>

#include "ledscape/ledscape.h"
#include "lib/cesanta/mongoose.h"
#include "opc/animation.h"
#include "opc/color.h"
#include "opc/ledscape_driver.h"
#include "opc/server-config.h"
#include "opc/server-error.h"
#include "opc/server-pru.h"
#include "util.h"

// TODO:
// Server:
//  - ip-stack Agnostic socket stuff
//  - UDP receiver
// Config:
//  - White-balance, curve adjustment
//  - Respecting interpolation and dithering settings

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Threads
//void *render_thread(void *threadarg);
void *udp_server_thread(void *threadarg);
void *tcp_server_thread(void *threadarg);
void *e131_server_thread(void *threadarg);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Data

// Global thread handles
typedef struct {
  pthread_t handle;
  bool enabled;
  bool running;
} thread_state_lt;

static struct {
  thread_state_lt tcp_server_thread;
  thread_state_lt udp_server_thread;
  thread_state_lt e131_server_thread;
} g_threads;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main()
static struct option long_options[] = {
    {"tcp-port", required_argument, NULL, 'p'},
    {"udp-port", required_argument, NULL, 'P'},

    {"e131-port", required_argument, NULL, 'e'},

    {"count", required_argument, NULL, 'c'},
    {"strip-count", required_argument, NULL, 's'},
    {"dimensions", required_argument, NULL, 'd'},

    {"channel-order", required_argument, NULL, 'o'},

    {"demo-mode", required_argument, NULL, 'D'},

    {"no-interpolation", no_argument, NULL, 'i'},
    {"no-dithering", no_argument, NULL, 't'},
    {"no-lut", no_argument, NULL, 'l'},

    {"help", no_argument, NULL, 'h'},

    {"lum_power", required_argument, NULL, 'L'},

    {"red_bal", required_argument, NULL, 'r'},
    {"green_bal", required_argument, NULL, 'g'},
    {"blue_bal", required_argument, NULL, 'b'},

    {"pru0_mode", required_argument, NULL, '0'},
    {"pru1_mode", required_argument, NULL, '1'},

    {"mode", required_argument, NULL, 'm'},
    {"mapping", required_argument, NULL, 'M'},

    {"config", required_argument, NULL, 'C'},

    {NULL, 0, NULL, 0}};

void set_pru_mode_and_mapping_from_legacy_output_mode_name(
    server_config_t *server_config, const char *input) {
  if (strcasecmp(input, "NOP") == 0) {
    server_config->output_mode_name == "nop";
    server_config->output_mapping_name = "original-ledscape";
  } else if (strcasecmp(input, "DMX") == 0) {
    server_config->output_mode_name = "dmx";
    server_config->output_mapping_name = "original-ledscape";
  } else if (strcasecmp(input, "WS2801") == 0) {
    server_config->output_mode_name = "ws2801";
    server_config->output_mapping_name = "original-ledscape";
  } else if (strcasecmp(input, "WS2801_NEWPINS") == 0) {
    server_config->output_mode_name = "ws2801";
    server_config->output_mapping_name = "rgb-123-v2";
  } else /*if (strcasecmp(input, "WS281x") == 0)*/ {
    // The default case is to use ws281x
    server_config->output_mode_name = "ws281x";
    server_config->output_mapping_name = "original-ledscape";
  }

  fprintf(stderr,
          "WARNING: PRU mode set using legacy -0 or -1 flags; please update to "
          "use --mode and --mapping.\n"
          "   '%s' interpreted as mode '%s' and mapping '%s'\n",
          input, server_config->output_mode_name.c_str(),
          server_config->output_mapping_name.c_str());
}

void print_usage(char **argv) {
  printf("Usage: %s ", argv[0]);

  int option_count = sizeof(long_options) / sizeof(struct option);
  for (int option_index = 0; option_index < option_count; option_index++) {
    struct option option_info = long_options[option_index];

    if (option_info.name != NULL) {
      if (option_info.has_arg == required_argument) {
        printf("[--%s <val> | -%c <val>] ", option_info.name, option_info.val);
      } else if (option_info.has_arg == optional_argument) {
        printf("[--%s[=<val>] | -%c[<val>] ", option_info.name,
               option_info.val);
      } else {
        printf("[--%s | -%c] ", option_info.name, option_info.val);
      }
    }
  }

  printf("\n");
}

void handle_args(int argc, char **argv, server_config_t *server_config,
                 std::string *config_file) {
  extern char *optarg;

  int opt;
  while ((opt = getopt_long(argc, argv, "p:P:c:s:d:D:o:ithlL:r:g:b:0:1:m:M:",
                            long_options, NULL)) != -1) {
    switch (opt) {
    case 'p': {
      server_config->tcp_port = (uint16_t)atoi(optarg);
    } break;

    case 'P': {
      server_config->udp_port = (uint16_t)atoi(optarg);
    } break;

    case 'e': {
      server_config->e131_port = (uint16_t)atoi(optarg);
    } break;

    case 'c': {
      server_config->leds_per_strip = (uint32_t)atoi(optarg);
    } break;

    case 's': {
      server_config->used_strip_count = (uint32_t)atoi(optarg);
    } break;

    case 'd': {
      int width = 0, height = 0;

      if (sscanf(optarg, "%dx%d", &width, &height) == 2) {
        server_config->leds_per_strip = (uint32_t)(width * height);
      } else {
        printf("Invalid argument for -d; expected NxN; actual: %s", optarg);
        exit(EXIT_FAILURE);
      }
    } break;

    case 'D': {
      server_config->demo_mode = demo_mode_from_string(optarg);
    } break;

    case 'o': {
      server_config->color_channel_order =
          color_channel_order_from_string(optarg);
    } break;

    case 'i': {
      server_config->interpolation_enabled = false;
    } break;

    case 't': {
      server_config->dithering_enabled = false;
    } break;

    case 'l': {
      server_config->lut_enabled = false;
    } break;

    case 'L': {
      server_config->lum_power = (float)atof(optarg);
    } break;

    case 'r': {
      server_config->white_point.red = (float)atof(optarg);
    } break;

    case 'g': {
      server_config->white_point.green = (float)atof(optarg);
    } break;

    case 'b': {
      server_config->white_point.blue = (float)atof(optarg);
    } break;

    case '0': {
      set_pru_mode_and_mapping_from_legacy_output_mode_name(server_config,
                                                            optarg);
    } break;

    case '1': {
      set_pru_mode_and_mapping_from_legacy_output_mode_name(server_config,
                                                            optarg);
    } break;

    case 'm': {
      server_config->output_mode_name = optarg;
    } break;

    case 'M': {
      server_config->output_mapping_name = optarg;
    } break;

    case 'C': {
      *config_file = optarg;

      if (read_config_file(config_file->c_str(), server_config) >= 0) {
        fprintf(stderr, "Loaded config file from %s.\n",
                config_file->c_str());
      } else {
        fprintf(stderr, "Config file not loaded: %s\n", opc_server_get_error().c_str());
      }
    } break;

    case 'h': {
      print_usage(argv);

      printf("\n");

      int option_count = sizeof(long_options) / sizeof(struct option);
      for (int option_index = 0; option_index < option_count; option_index++) {
        struct option option_info = long_options[option_index];
        if (option_info.name != NULL) {
          if (option_info.has_arg == required_argument) {
            printf("--%s <val>, -%c <val>\n\t", option_info.name,
                   option_info.val);
          } else if (option_info.has_arg == optional_argument) {
            printf("--%s[=<val>], -%c[<val>]\n\t", option_info.name,
                   option_info.val);
          } else {
            printf("--%s, -%c\n", option_info.name, option_info.val);
          }

          switch (option_info.val) {
          case 'p':
            printf("The TCP port to listen for OPC data on");
            break;
          case 'P':
            printf("The UDP port to listen for OPC data on");
            break;
          case 'e':
            printf("The UDP port to listen for e131 data on");
            break;
          case 'c':
            printf("The number of pixels connected to each output channel");
            break;
          case 's':
            printf("The number of used output channels (improves performance "
                   "by not interpolating/dithering unused channels)");
            break;
          case 'd':
            printf("Alternative to --count; specifies pixel count as a "
                   "dimension, e.g. 16x16 (256 pixels)");
            break;
          case 'D':
            printf("Configures the idle (demo) mode which activates when no "
                   "data arrives for more than 5 seconds. Modes:\n");
            printf("\t- none   Do nothing; leaving LED colors as they were\n");
            printf("\t- black  Turn off all LEDs");
            printf("\t- fade   Display a rainbow fade across all LEDs\n");
            printf(
                "\t- id     Send the channel index as all three color values "
                "or 0xAA (0b10101010) if channel and pixel index are equal");
            break;
          case 'o':
            printf("Specifies the color channel output order (RGB, RBG, GRB, "
                   "GBR, BGR or BRG); default is BRG.");
            break;
          case 'i':
            printf("Disables interpolation between frames (choppier output but "
                   "improves performance)");
            break;
          case 't':
            printf("Disables dithering (choppier output but improves "
                   "performance)");
            break;
          case 'l':
            printf("Disables luminance correction (lower color values appear "
                   "brighter than they should)");
            break;
          case 'L':
            printf("Sets the exponent of the luminance power function to the "
                   "given floating point value (default 2)");
            break;
          case 'r':
            printf("Sets the red balance to the given floating point number "
                   "(0-1, default .9)");
            break;
          case 'g':
            printf("Sets the red balance to the given floating point number "
                   "(0-1, default 1)");
            break;
          case 'b':
            printf("Sets the red balance to the given floating point number "
                   "(0-1, default 1)");
            break;
          case '0':
            printf("[deprecated] Sets the PRU0 program. Use --mode and "
                   "--mapping instead.");
            break;
          case '1':
            printf("[deprecated] Sets the PRU1 program. Use --mode and "
                   "--mapping instead.");
            break;
          case 'm':
            printf("Sets the output mode:\n");
            printf(
                "\t- nop      Disable output; can be useful for debugging\n");
            printf("\t- ws281x   WS2811/WS2812 output format\n");
            printf("\t- ws2801   WS2801-compatible 8-bit SPI output. Supports "
                   "24 channels of output with pins in a DATA/CLOCK "
                   "configuration.\n");
            printf(
                "\t- dmx      DMX compatible output (does not support RDM)\n");
            break;
          case 'M':
            printf("Sets the pin mapping used:\n");
            printf("\toriginal-ledscape: Original LEDscape pinmapping. Used on "
                   "older RGB-123 capes.\n");
            printf("\trgb-123-v2: RGB-123 mapping for new capes\n");
            break;
          case 'C':
            printf("Specifies a configuration file to use and creates it if it "
                   "does not already exist.\n");
            printf("\tIf used with other options, options are parsed in order. "
                   "Options before --config are overwritten\n");
            printf("\tby the config file, and options afterwards will be saved "
                   "to the config file.\n");
            break;
          case 'h':
            printf("Displays this help message");
            break;
          default:
            printf("Undocumented option: %c\n", option_info.val);
          }

          printf("\n");
        }
      }
      printf("\n");
      exit(EXIT_SUCCESS);
    }

    default:
      printf("Invalid option: %c\n\n", opt);
      print_usage(argv);
      printf("\nUse -h or --help for more information\n\n");
      exit(EXIT_FAILURE);
    }
  }
}

void validate_server_config_or_die(server_config_t* server_config) {
  std::string validation_output;
  if (validate_server_config(server_config, &validation_output) != 0) {
    die("ERROR: Configuration failed validation:\n%s", validation_output.c_str());
  }
}

int main(int argc, char **argv) {

  server_config_t server_config;
  init_server_config(&server_config);

  std::string config_file;
  handle_args(argc, argv, &server_config, &config_file);
  print_server_config(stderr, &server_config);
  validate_server_config_or_die(&server_config);

  // Save the config file if specified
  // TODO(gsasha): it looks that if config name is given, it is read and
  // then immediately written back. Seriously?
  if (config_file.size() > 0) {
    if (write_config_file(config_file.c_str(), &server_config) >= 0) {
      fprintf(stderr, "Config file written to %s\n", config_file.c_str());
    } else {
      fprintf(stderr, "Failed to write to config file %s: %s\n",
              config_file.c_str(), opc_server_get_error().c_str());
    }
  }

  fprintf(stderr,
          "[main] Starting server on ports (tcp=%d, udp=%d) for %d pixels on "
          "%d strips\n",
          server_config.tcp_port, server_config.udp_port,
          server_config.leds_per_strip, LEDSCAPE_NUM_STRIPS);

  bzero(&g_threads, sizeof(g_threads));
/*
  pthread_create(&g_threads.udp_server_thread.handle, NULL, udp_server_thread,
                 &g_runtime_state);
  pthread_create(&g_threads.tcp_server_thread.handle, NULL, tcp_server_thread,
                 &g_runtime_state);
  pthread_create(&g_threads.e131_server_thread.handle, NULL, e131_server_thread,
                 &g_runtime_state);
*/

  LedscapeDriver driver(server_config);
  Animation animation(&driver);
  animation.StartThread();
  animation.JoinThread();

  pthread_exit(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OPC Protocol Structures

typedef struct {
  uint8_t channel;
  uint8_t command;
  uint8_t len_hi;
  uint8_t len_lo;
} opc_cmd_t;

typedef enum {
  OPC_SYSID_FADECANDY = 1,

  // Pending approval from the OPC folks
  OPC_SYSID_LEDSCAPE = 2
} opc_system_id_t;

typedef enum { OPC_LEDSCAPE_CMD_GET_CONFIG = 1 } opc_ledscape_cmd_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// e131 Server
//

// From
// http://atastypixel-blog-content.s3.amazonaws.com/blog/wp-content/uploads/2010/05/multicast_sample.c
int join_multicast_group_on_all_ifaces(const int sock_fd,
                                       const char *group_ip) {
  // Obtain list of all network interfaces
  struct ifaddrs *addrs;

  if (getifaddrs(&addrs) < 0) {
    // Error occurred
    return -1;
  }

  // Loop through interfaces, selecting those AF_INET devices that support
  // multicast, but aren't loopback or point-to-point
  const struct ifaddrs *cursor = addrs;
  int joined_count = 0;
  while (cursor != NULL) {
    if (cursor->ifa_addr->sa_family == AF_INET &&
        !(cursor->ifa_flags & IFF_LOOPBACK) &&
        !(cursor->ifa_flags & IFF_POINTOPOINT) &&
        (cursor->ifa_flags & IFF_MULTICAST)) {
      // Prepare multicast group join request
      struct ip_mreq multicast_req;
      memset(&multicast_req, 0, sizeof(multicast_req));
      multicast_req.imr_multiaddr.s_addr = inet_addr(group_ip);
      multicast_req.imr_interface =
          ((struct sockaddr_in *)cursor->ifa_addr)->sin_addr;

      // Workaround for some odd join behaviour: It's perfectly legal to join
      // the same group on more than one interface, and up to 20 memberships may
      // be added to the same socket (see ip(4)), but for some reason, OS X
      // spews
      // 'Address already in use' errors when we actually attempt it.	As a
      // workaround, we can 'drop' the membership first, which would normally
      // have no effect, as we have not yet joined on this interface. However,
      // it enables us to perform the subsequent join, without dropping prior
      // memberships.
      setsockopt(sock_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast_req,
                 sizeof(multicast_req));

      // Join multicast group on this interface
      if (setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_req,
                     sizeof(multicast_req)) >= 0) {
        printf("[e131] Joined multicast group %s on %s\n", group_ip,
               inet_ntoa(((struct sockaddr_in *)cursor->ifa_addr)->sin_addr));
        joined_count++;
      } else {
        // Error occurred
        freeifaddrs(addrs);
        return -1;
      }
    }

    cursor = cursor->ifa_next;
  }

  freeifaddrs(addrs);

  return joined_count;
}

void *e131_server_thread(void *runtime_state_ptr) {
  runtime_state_ptr = runtime_state_ptr;
#if 0
  runtime_state_t *runtime_state = (runtime_state_t *)runtime_state_ptr;
  server_config_t *server_config = &runtime_state->server_config;
  render_state_t *render_state = &runtime_state->render_state;

  // Disable if given port 0
  if (server_config->e131_port == 0) {
    fprintf(stderr, "[e131] Not starting e131 server; Port is zero.\n");
    pthread_exit(NULL);
    return NULL;
  }

  fprintf(stderr, "[e131] Starting UDP server on port %d\n",
          server_config->e131_port);
  uint8_t packet_buffer[65536]; // e131 packet buffer

  const int sock = socket(AF_INET6, SOCK_DGRAM, 0);

  if (sock < 0)
    die("[e131] socket failed: %s\n", strerror(errno));

  struct sockaddr_in6 addr;
  bzero(&addr, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;
  addr.sin6_port = htons(server_config->e131_port);

  if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "[e131] bind port %d failed: %s\n",
            server_config->e131_port, strerror(errno));
    pthread_exit(NULL);
    return NULL;
  }

  int32_t last_seq_num = -1;

  // Bind to multicast
  if (join_multicast_group_on_all_ifaces(sock, "239.255.0.0") < 0) {
    fprintf(stderr, "[e131] failed to bind to multicast addresses\n");
  }

  uint8_t *dmx_buffer = NULL;
  uint32_t dmx_buffer_size = 0;

  uint32_t packets_since_update = 0;
  uint32_t frame_counter_at_last_update = render_state->frame_counter;

  while (1) {
    const ssize_t received_packet_size =
        recv(sock, packet_buffer, sizeof(packet_buffer), 0);
    if (received_packet_size < 0) {
      fprintf(stderr, "[e131] recv failed: %s\n", strerror(errno));
      continue;
    }

    // Ensure the buffer
    uint32_t leds_per_strip = server_config->leds_per_strip;
    uint32_t led_count = server_config->leds_per_strip * LEDSCAPE_NUM_STRIPS;

    if (dmx_buffer == NULL || dmx_buffer_size != led_count) {
      if (dmx_buffer != NULL)
        free(dmx_buffer);
      dmx_buffer_size = led_count * sizeof(buffer_pixel_t);
      dmx_buffer = malloc(dmx_buffer_size);
    }

    // Packet should be at least 126 bytes for the header
    if (received_packet_size >= 126) {
      int32_t current_seq_num = packet_buffer[111];

      if (last_seq_num == -1 || current_seq_num >= last_seq_num ||
          (last_seq_num - current_seq_num) > 64) {
        last_seq_num = current_seq_num;

        // 1-based DMX universe
        uint16_t dmx_universe_num =
            ((uint16_t)packet_buffer[113] << 8) | packet_buffer[114];

        if (dmx_universe_num >= 1 && dmx_universe_num <= 48) {
          uint16_t ledscape_channel_num = dmx_universe_num - 1;
          // Data OK
          //   set_next_frame_single_channel_data(
          //       ledscape_channel_num,
          //       packet_buffer + 126,
          //       received_packet_size - 126, true);

          memcpy(dmx_buffer + ledscape_channel_num * leds_per_strip *
                                  sizeof(buffer_pixel_t),
                 packet_buffer + 126,
                 min((uint)(received_packet_size - 126),
                     led_count * sizeof(buffer_pixel_t)));

          set_next_frame_data(render_state, dmx_buffer,
                              dmx_buffer_size * sizeof(buffer_pixel_t), true);
        } else {
          fprintf(stderr, "[e131] DMX universe %d out of bounds [1,48] \n",
                  dmx_universe_num);
        }
      } else {
        // Out of order sequence packet
        fprintf(stderr, "[e131] out of order packet; current %d, old %d \n",
                current_seq_num, last_seq_num);
      }
    } else {
      fprintf(stderr, "[e131] packet too small: %d < 126 \n",
              received_packet_size);
    }

    // Increment counter
    packets_since_update++;
    if (render_state->frame_counter != frame_counter_at_last_update) {
      packets_since_update = 0;
      frame_counter_at_last_update = render_state->frame_counter;
    }

    if (packets_since_update >= LEDSCAPE_NUM_STRIPS) {
      // Force an update here
      while (render_state->frame_counter == frame_counter_at_last_update)
        usleep(1e3 /* 1ms */);
    }
  }
#endif
  pthread_exit(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDP Server
//

void *udp_server_thread(void *runtime_state_ptr) {
runtime_state_ptr = runtime_state_ptr;
#if 0
  runtime_state_t *runtime_state = (runtime_state_t *)runtime_state_ptr;
  server_config_t *server_config = &runtime_state->server_config;
  render_state_t *render_state = &runtime_state->render_state;

  // Disable if given port 0
  if (server_config->udp_port == 0) {
    fprintf(stderr, "[udp] Not starting UDP server; Port is zero.\n");
    pthread_exit(NULL);
    return NULL;
  }

  uint32_t required_packet_size =
      server_config->used_strip_count * server_config->leds_per_strip * 3 +
      sizeof(opc_cmd_t);
  if (required_packet_size > 65507) {
    fprintf(stderr,
            "[udp] OPC command for %d LEDs cannot fit in UDP packet. Use "
            "--count or --strip-count to reduce the number of required LEDs, "
            "or disable UDP server with --udp-port 0\n",
            server_config->used_strip_count * server_config->leds_per_strip);
    pthread_exit(NULL);
    return NULL;
  }

  fprintf(stderr, "[udp] Starting UDP server on port %d\n",
          server_config->udp_port);
  uint8_t buf[65536];

  const int sock = socket(AF_INET6, SOCK_DGRAM, 0);

  if (sock < 0)
    die("[udp] socket failed: %s\n", strerror(errno));

  struct sockaddr_in6 addr;
  bzero(&addr, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;
  addr.sin6_port = htons(server_config->udp_port);

  if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
    die("[udp] bind port %d failed: %s\n", server_config->udp_port,
        strerror(errno));

  while (1) {
    const ssize_t rc = recv(sock, buf, sizeof(buf), 0);
    if (rc < 0) {
      fprintf(stderr, "[udp] recv failed: %s\n", strerror(errno));
      continue;
    }

    // Enough data for an OPC command header?
    if (rc >= (int)sizeof(opc_cmd_t)) {
      opc_cmd_t *cmd = (opc_cmd_t *)buf;
      const size_t cmd_len = cmd->len_hi << 8 | cmd->len_lo;

      uint8_t *opc_cmd_payload = ((uint8_t *)buf) + sizeof(opc_cmd_t);

      // Enough data for the entire command?
      if (rc >= (int)(sizeof(opc_cmd_t) + cmd_len)) {
        if (cmd->command == 0) {
          set_next_frame_data(render_state, opc_cmd_payload, cmd_len, true);
        } else if (cmd->command == 255) {
          // System specific commands
          const uint16_t system_id =
              opc_cmd_payload[0] << 8 | opc_cmd_payload[1];

          if (system_id == OPC_SYSID_LEDSCAPE) {
            const opc_ledscape_cmd_id_t ledscape_cmd_id = opc_cmd_payload[2];

            if (ledscape_cmd_id == OPC_LEDSCAPE_CMD_GET_CONFIG) {
              warn("[udp] WARN: Config request request received but not "
                   "supported on UDP.\n");
            } else {
              warn("[udp] WARN: Received command for unsupported LEDscape "
                   "Command: %d\n",
                   (int)ledscape_cmd_id);
            }
          } else {
            warn("[udp] WARN: Received command for unsupported system-id: %d\n",
                 (int)system_id);
          }
        }
      }
    }
  }

#endif
  pthread_exit(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TCP Server
#if 0
static void event_handler(struct mg_connection *conn, int ev,
                          void *runtime_state_ptr) {
  runtime_state_t *runtime_state = (runtime_state_t *)runtime_state_ptr;
  server_config_t *server_config = &runtime_state->server_config;
  render_state_t *render_state = &runtime_state->render_state;

  struct mbuf *io = &conn->recv_mbuf; // IO buffer that holds received message
  switch (ev) {
  case MG_EV_RECV: {
    // Enough data for an OPC command header?
    if (io->len >= sizeof(opc_cmd_t)) {
      opc_cmd_t *cmd = (opc_cmd_t *)io->buf;
      const size_t cmd_len = cmd->len_hi << 8 | cmd->len_lo;

      uint8_t *opc_cmd_payload = ((uint8_t *)io->buf) + sizeof(opc_cmd_t);

      // Enough data for the entire command?
      if (io->len >= sizeof(opc_cmd_t) + cmd_len) {
        if (cmd->command == 0) {
          set_next_frame_data(render_state, opc_cmd_payload, cmd_len, true);
        } else if (cmd->command == 255) {
          // System specific commands
          const uint16_t system_id =
              opc_cmd_payload[0] << 8 | opc_cmd_payload[1];

          if (system_id == OPC_SYSID_LEDSCAPE) {
            const opc_ledscape_cmd_id_t ledscape_cmd_id = opc_cmd_payload[2];

            if (ledscape_cmd_id == OPC_LEDSCAPE_CMD_GET_CONFIG) {
              warn("[tcp] Responding to config request\n");
              char json[4096];
              server_config_to_json(json, sizeof(json), server_config);
              mg_send(conn, json, strlen(json) + 1);
            } else {
              warn("[tcp] WARN: Received command for unsupported LEDscape "
                   "Command: %d\n",
                   (int)ledscape_cmd_id);
            }
          } else {
            warn("[tcp] WARN: Received command for unsupported system-id: %d\n",
                 (int)system_id);
          }
        }

        // Removed the processed command from the buffer
        mbuf_remove(io, sizeof(opc_cmd_t) + cmd_len);
      }
    }

    // Fallback to handle misformed data. Clear the io buffer if we have more
    // than 100k waiting.
    if (io->len > 1e5) {
      mbuf_remove(io, io->len);
    }
  } break;

  case MG_EV_ACCEPT: {
    char buffer[INET6_ADDRSTRLEN];
    mg_sock_to_str(conn->sock, buffer, sizeof(buffer), 1);
    printf("[tcp] Connection from %s\n", buffer);
  } break;

  default:
    break; // We ignore all other events
  }
}
#endif

void *tcp_server_thread(void *runtime_state_ptr) {
runtime_state_ptr = runtime_state_ptr;
#if 0
  runtime_state_t *runtime_state = (runtime_state_t *)runtime_state_ptr;
  server_config_t *server_config = &runtime_state->server_config;

  // Disable if given port 0
  if (server_config->tcp_port == 0) {
    fprintf(stderr, "[tcp] Not starting TCP server; Port is zero.\n");
    pthread_exit(NULL);
    return NULL;
  }

  struct mg_mgr mgr;
  mg_mgr_init(&mgr, NULL);

  char s_bind_addr[128];
  sprintf(s_bind_addr, "tcp://:%d", server_config->tcp_port);

  struct mg_bind_opts opts;
  memset(&opts, 0, sizeof(opts));
  const char *err = NULL;
  opts.error_string = &err;

  // Initialize server and open listening port
  struct mg_connection *connection =
      mg_bind_opt(&mgr, s_bind_addr, MG_CB(event_handler, runtime_state_ptr), opts);
  if (connection == NULL) {
    printf("[tcp] Failed to bind to port %s: %s\n", s_bind_addr, err);
    exit(-1);
  }

  printf("[tcp] Starting TCP server on %s\n", s_bind_addr);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);
#endif
  pthread_exit(NULL);
}

