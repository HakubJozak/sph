#!/usr/bin/env ruby

require 'singleton'
require 'rubygems'
require 'bundler'
Bundler.require(:default, :debug)

# require_all 'lib/**/*.rb'


class Win < Gosu::Window

  def initialize()
    super(600, 600, false)
    $window = self
  end

  def draw
    Particle.new(30,30).draw
  end

  def update
    if button_down? Gosu::KbEscape then close end
  end

end

class Particle

  def initialize(x,y)
    @x, @y = x, y
    @img = Gosu::Image.new($window, "particle.png", false)
  end

  def draw
    @img.draw_rot(@x, @y, 0, 0)
  end

end



Win.new.show

