#help_index "Graphics"

U0 GrInit1()
{
  DefineLstLoad("ST_SPRITE_ELEM_TYPES",
	"End\0Color\0Dither Color\0"
	"Thick\0Planar Symmetry\0Transform On\0Transform Off\0Shift\0"
	"Point\0PolyPoint\0Line\0PolyLine\0Rect\0Rotated Rect\0"
	"Circle\0Ellipse\0Polygon\0BSpline2\0BSpline2 Closed\0"
	"BSpline3\0BSpline3 Closed\0"
	"Flood Fill\0Flood Fill Not Color\0BitMap\0Mesh\0Shiftable Mesh\0"
	"Arrow\0Text\0Text Box\0Text Diamond\0");
  DefineLstLoad("ST_SPRITE_ELEM_CODES",
	"End\0Color\0Color\0"
	"Thick\0PlanarSymmetry\0Transform\0Transform\0Shift\0"
	"Pt\0PolyPt\0Line\0PolyLine\0Rect\0Rect\0"
	"Circle\0Ellipse\0Polygon\0BSpline2\0BSpline2\0BSpline3\0BSpline3\0"
	"FloodFill\0FloodFill\0BitMap\0Mesh\0Mesh\0"
	"Arrow\0Text\0TextBox\0TextDiamond\0");
}
GrInit1;
