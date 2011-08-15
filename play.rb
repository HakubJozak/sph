#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'singleton'
require 'rubygems'
require 'bundler'
Bundler.require(:default, :debug)


require 'ostruct'
require './mouse'
# require_all 'lib/**/*.rb'


$h = 50
STEP = $h
W = 600
H = 600


class Vector2D
  attr_accessor :x,:y

  def initialize(x,y)
    @x,@y = x,y
  end

  def +(b)
    Vector2D.new(b.x + @x, b.y + @y)
  end

  def -(b)
    Vector2D.new(@x - b.x, @y - b.y)
  end

  def *(alpha)
    Vector2D.new(alpha * @x, alpha * @y)
  end

  def to_s
    "(#{@x}, #{@y})"
  end

  def distance_squared(b)
    dx =  (b.x - @x)
    dy =  (b.y - @y)
    (dx * dx + dy * dy)
  end

end



class Particle

  attr_accessor :pos, :density, :force, :v

  def initialize(x,y)
    @m = 1.0
    @density = 1.0

    @pos = Vector2D.new(x, y)
    @v = Vector2D.new(0.0,0.0)
    @force = Vector2D.new(0.0,0.0)

    @img = Gosu::Image.new($window, make_circle(2))
  end

  def neighbour?(p)
    self.pos.distance_squared(p.pos) <= ($h * $h)
  end

  def calc_density
    @density = neighbours.inject(0) do |rho, n|
      x = 1.0 - (Math::sqrt(pos.distance_squared(n.pos)) / $h)
      rho + x*x
    end
  end

  def calc_pressure_force
    rest_density = 1.0
    k = 1000.0

    neighbours.each do |neighbour|
      distance = Math::sqrt(pos.distance_squared(neighbour.pos))

      q = 1.0 - distance / $h
      press = k *(self.density + neighbour.density - (2 * rest_density))

      c = (press / distance * 0.5)
      r = self.pos - neighbour.pos


      f = Vector2D.new( r.x * c, r.y * c)
      @force = @force - f
    end
  end

  def neighbours
    $particles.select { |p| p != self && self.neighbour?(p) }
  end

  def make_circle(r)
    circle = Magick::Image.new(2 * r, 2 * r) { self.background_color = 'none' }
    gc = Magick::Draw.new
    gc.fill('red').circle(r, r, r, 0)
    gc.draw(circle)
    circle
  end

  def draw
    @img.draw_rot(@pos.x, @pos.y, 0, 0)
  end

  def to_s
    puts "---------------------------"
    puts "pos = #{@pos}"
    puts "v = #{@v}"
    puts "density = #{@density}"
    puts "force = #{@force}"
  end

end


module Timer

  # TODO: replace by threading
  def every(millis, &block)
    now = Gosu::milliseconds
    @last_yield ||= now - millis

    if (@last_yield + millis) <= now
      yield
      @last_yield = now
    end
  end

end



class Win < Gosu::Window

  include Timer

  def initialize()
    super(W, H, false)
    $window = self
    @mouse = Mouse.new

    $particles = []

    (0..H).step(STEP) do |y|
      (0..W).step(STEP) do |x|
        # misplaced particle to change the initial density
        oy = y
        ox = x
        ox += 30 if x == 200 && y == 200

        $particles << Particle.new(ox,oy)
      end
    end
  end


  def draw
    @mouse.draw

    if button_down?(Gosu::MsLeft)
      nearest = $particles.min do |a,b|
        a.pos.distance_squared(@mouse.pos) <=> b.pos.distance_squared(@mouse.pos)
      end

      puts nearest.inspect
    end

    $particles.each { |p| p.draw }
  end


  def update
    every(100) do
      $particles.each(&:calc_density)
      $particles.each(&:calc_pressure_force)

      dt = 0.01

      $particles.each do |p|
        p.v = p.v + p.force * dt
        p.pos = p.pos + p.v * dt
      end
    end

    close if button_down?(Gosu::KbEscape)
  end

end



Win.new.show

