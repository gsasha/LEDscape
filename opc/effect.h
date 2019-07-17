class Effect {
public:
  virtual ~Effect() = default;

  virtual void RenderFrame() = 0;
};
