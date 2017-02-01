"""A module that should help create the affine transformations you would use
for transforming GnomeCanvas items.

As a little background, an affine transformation is a linear transformation
and a translation.  The linear transformationcan be represented by a matrix
and the translation by a vector.  So a general affine transformation looks
like this:
   [ x ] -> [ a b ][ x ] + [ e ]
   [ y ]    [ c d ][ y ]   [ f ]

As you can see, the affine transformation is defined by the six numbers in
the matrix and column vector.  For the GnomeCanvas item functions, affine
transformations are represented by a flat sequence of the form
  (a, c, b, d, e, f)
  
This module attempts to make it easy to create complex affine transformations
from simpler ones like rotation, scaling and translation.
"""

import math

# These functions create base affine transformations that can be used to build
# more complex ones.

def identity():
	"""The identity affine transformation"""
	return (1, 0, 0, 1, 0, 0)

def scale(sx=1, sy=1):
	"""Scale by a factor of sx in the X direction and sy in Y direction"""
	return (sx, 0, 0, sy, 0, 0)

def rotate(radians=None, degrees=0):
	"""Rotate in either degrees or radians"""
	if not radians: radians = degrees * math.pi / 180.0
	s = math.sin(radians)
	c = math.cos(radians)
	return (c, s, -s, c, 0, 0)

def shear(xshear=0, yshear=0):
	"""Creates a shear transformation in either X and/or Y directoion"""
	return (1, yshear, xshear, 1, 0, 0)

def translate(tx=0, ty=0):
	"""Translate by (tx,ty)"""
	return (1, 0, 0, 1, tx, ty)


# These functions can be used to combine or manipulate other affine
# tranformations

def compose(a1, *affines):
	"""Compose a number of affine transformations together

	If the affines a1,a2,a3 are passed as arguments, then the resulting
	affine A will give A(x) == a1(a2(a3(x)))"""
	if affines == (): return a1
	a2 = apply(compose, affines)
	return (a1[0]*a2[0] + a1[2]*a2[1],
		a1[1]*a2[0] + a1[3]*a2[1],
		a1[0]*a2[2] + a1[2]*a2[3],
		a1[1]*a2[2] + a1[3]*a2[3],
		a1[0]*a2[4] + a1[2]*a2[5] + a1[4],
		a1[1]*a2[4] + a1[3]*a2[5] + a1[5])

def invert(aff):
	det_inv = 1 / (aff[0]*aff[3] - aff[1]*aff[2])
	return (aff[3]  * det_inv,
		-aff[1] * det_inv,
		-aff[2] * det_inv,
		aff[0]  * det_inv,
		(aff[2]*aff[5] - aff[3]*aff[4]) * det_inv,
		(aff[1]*aff[4] - aff[0]*aff[5]) * det_inv)

