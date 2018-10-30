## Not your Dad's blackboard.

*The blackboard's surface grows exponentially with distance from the centre,
instead of the usual linear growth.* (Specifically, the blackboard's surface is
a 2-dimensional Riemannian manifold with uniform negative curvature.  It is
rendered conformally as a Poincaré disk.) This allows for the drawing of trees,
graphs, and other novel . . . things that would not be possible on a regular
blackboard.

*Everything you have ever drawn on the infiniboard will be visible, at least at
some zoom level.* As the blackboard is panned, objects in the centre get
smaller as they approach the edge, and objects at the edge get larger as they
approach the centre. It is impossible for an object to go out of view, though
it may be reduced to an indecipherable pixel.

*Responsiveness over prettiness.* This is supposed to be a tool for
communication and problem solving in real time, just like a real blackboard.
Great care has been taken to ensure the blackboard is as responsive as humanly
or machinely possible. It's a little hard to tell, but mouse to screen response
is currently supposed to be 3ms best case and 1/f+3ms worst case, where f is
the monitor's refresh rate. I still observe lag with respect to the system
cursor, but that could just be X11 and its perpetually-broken-ass self. Which
reminds me.

*Will support Wayland.* In fact, it may already support wayland. Infiniboard is
implemented with glfw, so it should Just Work™. Testers are welcome.

Besides that, it's really just a regular blackboard.

## It's not ready yet! Don't eat it! Get out of the kitchen!

Unless you would brave a taste test. But mostly, don't complain that it doesn't
work, because that isn't news to me. Right now, it's just a pannable
background.  Drawing has not been implemented yet.  This is next on my todo
list after I take care of some dirty edge cases in the background drawing.

## TODO:

* dirty background edge cases
* drawing
* interpolate drawn segments with some sexy cubic splines.
* undo
* erase of every segment in the path of a point cursor
* clipped erase of everything under a finite-area erase cursor of variable
  size.
* use the background's periodic nature to deliver the illusion of an infinite
  background with a finite mesh. (Move the mesh back to the centre when the
  distance exceeds a certain amount, and do it in a way that is impossible to
  notice.)
* store drawn segments in tiles, so that the size of the blackboard is truly
  infinite, and to avert floating-point badness at great distances away from
  the origin.
* quantify mouse to screen response
* scons debug=0 is spectacularly broken in multiple ways
* touch screens and smart boards, pending acquisition of capable hardware.
* vulkan, pending acquisition of vulkan-capable hardware. If this ever
  *actually* happens, support for OpenGL and its perpetually-broken-ass self
  will probably be dropped.
* rust rewrite, pending vulkan and the arrival of some definitely-not-broken
  vulkan and glfw bindings for rust. Because holy actual shit, C is terrible.
  But there are worse things than C and one of them is debugging broken vulkan
  and glfw rust bindings.
* antialiasing, provided there is a negligible latency hit

## DEFINITELY-WILL-NOT-DO:

* directx
* macs. It might work ootb, but it will not be supported.
* Any operating system that uses the wrong path separator.
* eigen, or any other templatey C++ badness, but especially eigen. As
  rationale, I would show you literally any compile-time error message produced
  by that library or any other piece of templated C++ code, but you might go
  insane, H.  P.  Lovecraft style.  I do want to clean up the math, but I think
  rust is the way to go there.



 vi:fo=qat
