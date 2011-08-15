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

  # def +(b)
  #   Vector2D.new(b.x + @x, b.y + @y)
  # end

  def -(b)
    Vector2D.new(@x - b.x, @y - b.y)
  end

  # def *(alpha)
  #   Vector2D.new(alpha * @x, alpha * @y)
  # end

end



class Particle

  attr_accessor :x, :y, :density, :force_p

  def initialize(x,y)
    @x, @y = x, y

    @m = 1.0
    @density = 1.0
    @force_p = Vector2D.new(0.0,0.0)

    @img = Gosu::Image.new($window, make_circle(2))
  end

  # Squared distance between this particle and particle 'p'
  #
  def distance_squared(p)
    dx =  (p.x - @x)
    dy =  (p.y - @y)
    (dx * dx + dy * dy)
  end

  def neighbour?(p)
    distance_squared(p) <= ($h * $h)
  end

  def calc_density
    @density = neighbours.inject(0) do |rho, n|
      x = 1.0 - (Math::sqrt(distance_squared(n)) / $h)
      rho + x*x
    end
  end

  def calc_pressure_force
    rest_density = 1.0
    k = 1000.0

    neighbours.each do |neighbour|
      distance = Math::sqrt(distance_squared(neighbour))

      q = 1.0 - distance / $h
      press = k *(self.density + neighbour.density - (2 * rest_density))

      c = (press / distance * 0.5)

      f = Vector2D.new( (self.x - neighbour.x) * c, (self.y - neighbour.y) * c)
      @force_p = @force_p - f
    end
  end

  # Smoothing function
  #
  def a
  end

  # Kernel function
  #
  def w
  end

  def make_circle(r)
    circle = Magick::Image.new(2 * r, 2 * r) { self.background_color = 'none' }
    gc = Magick::Draw.new
    gc.fill('red').circle(r, r, r, 0)
    gc.draw(circle)
    circle
  end

  def draw
    @img.draw_rot(@x, @y, 0, 0)
  end

  private

  def neighbours
    $particles.select { |p| p != self && self.neighbour?(p) }
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
        a.distance_squared(@mouse) <=> b.distance_squared(@mouse)
      end

      puts nearest.inspect
    end

    $particles.each { |p| p.draw }
  end


  def update
    every(1000) do
      $particles.each(&:calc_density)
      $particles.each(&:calc_pressure_force)
    end

    close if button_down?(Gosu::KbEscape)
  end

end



Win.new.show

