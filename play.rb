#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'singleton'
require 'rubygems'
require 'bundler'
Bundler.require(:default, :debug)


require 'ostruct'
require './mouse'
# require_all 'lib/**/*.rb'


$h = 40
STEP = $h
W = 600
H = 600



class Particle

  attr_accessor :x, :y

  def initialize(x,y)
    @x, @y = x, y
    @ux, @uy = 0.0, 0.0

    @m = 1.0
    @density = 1.0
    @p_force = 1.0
    @viscosity_force = 1.0

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
    neighbours = $particles.reject { |p| self.neighbour?(p) }

    @density = neighbours.inject(0) do |rho, n|
      x = 1.0 - (distance_squared(n) / h)
      rho + x*x
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

end




class Win < Gosu::Window

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
      # init density of all particles
      # clear pressure-force of all particles
      # clear viscosity-force of all particles
      # clear colour-field-gradient of all particles
      # clear colour-field-laplacian of all particles

    $particles.each(&:calc_density)

    close if button_down?(Gosu::KbEscape)
  end

end



Win.new.show

