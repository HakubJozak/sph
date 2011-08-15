#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'singleton'
require 'rubygems'
require 'bundler'
Bundler.require(:default, :debug)

# require_all 'lib/**/*.rb'


$h = 40
STEP = $h
W = 600
H = 600



class Particle

  def initialize(x,y)
    @x, @y = x, y
    @ux, @uy = 0.0, 0.0

    @m = 1.0
    @density = 1.0
    @p_force = 1.0
    @viscosity_force = 1.0

    @img = Gosu::Image.new($window, make_circle(2))
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

    @particles = []

    (0..H).step(STEP) do |y|
      (0..W).step(STEP) do |x|
        # misplaced particle to change the initial density
        oy = y
        ox = x
        ox += 30 if x == 200 && y == 200

        @particles << Particle.new(ox,oy)
      end
    end
  end

  def draw
    @particles.each { |p| p.draw }
  end


  def update

      # init density of all particles
      # clear pressure-force of all particles
      # clear viscosity-force of all particles
      # clear colour-field-gradient of all particles
      # clear colour-field-laplacian of all particles

    @particles.each do

    end

    close if button_down?(Gosu::KbEscape)
  end

end



Win.new.show

