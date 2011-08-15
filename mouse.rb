class Mouse

  def initialize
    @size = 10
    @green = sketch('green')
    @red = sketch('red')
    super
  end

  def pos
    Vector2D.new(x,y)
  end

  def draw
    if $window.button_down? Gosu::MsLeft
      @image = @green
    else
      @image = @red
    end

    @image.draw $window.mouse_x - @size, $window.mouse_y - @size, 10000
  end

  def x; $window.mouse_x end
  def y; $window.mouse_y end

  private

  def sketch(color)
    canvas = Magick::Image.new(2 * @size + 1,2 * @size + 1, Magick::HatchFill.new('transparent','transparent'))
    pen = Magick::Draw.new
    pen.translate 10,10
    pen.stroke(color)
    pen.stroke_width 2
    pen.line 0,10, 10, 0
    pen.line 0,0, 10, 10
    pen.draw(canvas)
    Gosu::Image.new($window, canvas)
  end

end


