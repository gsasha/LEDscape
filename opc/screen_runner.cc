#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

class PixelRenderer : public Fl_Box {
public:
  PixelRenderer(int X, int Y, int W, int H, const char *L = 0)
      : Fl_Box(X, Y, W, H, L) {
    box(FL_FLAT_BOX);
    color(FL_DARK2);
  }
  void draw() override {
    Fl_Box::draw();
    fl_rectf(10, 10, 20, 20, FL_WHITE);
  }
};

int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(340, 180);
  /*
  Fl_Box *box = new Fl_Box(20,40,300,100,"Hello, World!");
  box->box(FL_UP_BOX);
  box->labelfont(FL_BOLD+FL_ITALIC);
  box->labelsize(36);
  box->labeltype(FL_SHADOW_LABEL);
  */
  PixelRenderer *pixel_renderer = new PixelRenderer(10, 10, 300, 100);
  pixel_renderer = pixel_renderer;

  // Fl_Image_Surface *img_surf = new Fl_Image_Surface(window->w(),
  // window->h());
  window->end();
  window->show(argc, argv);
  return Fl::run();
}

