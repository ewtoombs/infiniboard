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
or machinely possible, even on really old hardware. It's a little hard to tell,
but mouse to screen response is currently supposed to be 3ms best case and 1/f
+ 3ms worst case, where f is the monitor's refresh rate.

Besides that, it's really just a regular blackboard.

## It's not ready yet! Don't eat it! Get out of the kitchen!

Right now, it's just a pannable background. Drawing has not been implemented
yet. This is next on my todo list after I take care of some dirty edge cases in
the background drawing.

## TODO:

* dirty background edge cases
* drawing
* quantify mouse to screen response
* undo
* erase by segment
* clipped erase of everything under cursor.



 vi:fo=qat
