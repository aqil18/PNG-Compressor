/**
 * @file qtree.cpp
 * @description student implementation of QTree class used for storing image data
 *              CPSC 221 PA3
 *
 *              SUBMIT THIS FILE
 */

#include "qtree.h"
#include <iostream>

using namespace std;
/**
 * Constructor that builds a QTree out of the given PNG.
 * Every leaf in the tree corresponds to a pixel in the PNG.
 * Every non-leaf node corresponds to a rectangle of pixels
 * in the original PNG, represented by an (x,y) pair for the
 * upper left corner of the rectangle and an (x,y) pair for
 * lower right corner of the rectangle. In addition, the Node
 * stores a pixel representing the average color over the
 * rectangle.
 *
 * The average color for each node in your implementation MUST
 * be determined in constant time. HINT: this will lead to nodes
 * at shallower levels of the tree to accumulate some error in their
 * average color value, but we will accept this consequence in
 * exchange for faster tree construction.
 * Note that we will be looking for specific color values in our
 * autograder, so if you instead perform a slow but accurate
 * average color computation, you will likely fail the test cases!
 *
 * Every node's children correspond to a partition of the
 * node's rectangle into (up to) four smaller rectangles. The node's
 * rectangle is split evenly (or as close to evenly as possible)
 * along both horizontal and vertical axes. If an even split along
 * the vertical axis is not possible, the extra line will be included
 * in the left side; If an even split along the horizontal axis is not
 * possible, the extra line will be included in the upper side.
 * If a single-pixel-wide rectangle needs to be split, the NE and SE children
 * will be null; likewise if a single-pixel-tall rectangle needs to be split,
 * the SW and SE children will be null.
 *
 * In this way, each of the children's rectangles together will have coordinates
 * that when combined, completely cover the original rectangle's image
 * region and do not overlap.
 */
QTree::QTree(const PNG& imIn) {
	width = imIn.width();
	height = imIn.height();
	
	root = BuildNode(imIn, make_pair(0, 0), make_pair(width-1, height-1));
}

/**
 * Overloaded assignment operator for QTrees.
 * Part of the Big Three that we must define because the class
 * allocates dynamic memory. This depends on your implementation
 * of the copy and clear funtions.
 *
 * @param rhs The right hand side of the assignment statement.
 */
QTree& QTree::operator=(const QTree& rhs) {
	Clear();
	Copy(rhs);
	return *this;
}

/**
 * Render returns a PNG image consisting of the pixels
 * stored in the tree. may be used on pruned trees. Draws
 * every leaf node's rectangle onto a PNG canvas using the
 * average color stored in the node.
 *
 * For up-scaled images, no color interpolation will be done;
 * each rectangle is fully rendered into a larger rectangular region.
 *
 * @param scale multiplier for each horizontal/vertical dimension
 * @pre scale > 0
 */
PNG QTree::Render(unsigned int scale) const {
	PNG output =  PNG(width*scale, height*scale);
	Render(root, scale, output);
	return output;
}

/**
 *  Prune function trims subtrees as high as possible in the tree.
 *  A subtree is pruned (cleared) if all of the subtree's leaves are within
 *  tolerance of the average color stored in the root of the subtree.
 *  NOTE - you may use the distanceTo function found in RGBAPixel.h
 *  Pruning criteria should be evaluated on the original tree, not
 *  on any pruned subtree. (we only expect that trees would be pruned once.)
 *
 * You may want a recursive helper function for this one.
 *
 * @param tolerance maximum RGBA distance to qualify for pruning
 * @pre this tree has not previously been pruned, nor is copied from a previously pruned tree.
 */
void QTree::Prune(double tolerance) {
	Prune(root, tolerance);	
}

/**
 *  FlipHorizontal rearranges the contents of the tree, so that
 *  its rendered image will appear mirrored across a vertical axis.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  After flipping, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel wide rectangles have
 *  null eastern children
 *  (i.e. after flipping, a node's NW and SW pointers may be null, but
 *  have non-null NE and SE)
 * 
 *  You may want a recursive helper function for this one.
 */
void QTree::FlipHorizontal() {
	FlipHorizontal(root);
}

/**
 *  RotateCCW rearranges the contents of the tree, so that its
 *  rendered image will appear rotated by 90 degrees counter-clockwise.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  Note that this may alter the dimensions of the rendered image, relative
 *  to its original dimensions.
 *
 *  After rotation, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel tall or wide rectangles
 *  have null eastern or southern children
 *  (i.e. after rotation, a node's NW and NE pointers may be null, but have
 *  non-null SW and SE, or it may have null NW/SW but non-null NE/SE)
 *
 *  You may want a recursive helper function for this one.
 */
void QTree::RotateCCW() {
	int temp = width;
	width = height;
	height = temp;
	RotateCCW(root);	
}

/**
 * Destroys all dynamically allocated memory associated with the
 * current QTree object. Complete for PA3.
 * You may want a recursive helper function for this one.
 */
void QTree:: Clear() {
	Clear(root);	
}

/**
 * Copies the parameter other QTree into the current QTree.
 * Does not free any memory. Called by copy constructor and operator=.
 * You may want a recursive helper function for this one.
 * @param other The QTree to be copied.
 */
void QTree::Copy(const QTree& other) {
	if (other.root == nullptr) {
		root = nullptr;
		height = 0;
		width = 0;
	} else {
		height = other.height;
		width = other.width;
		CopyNodes(root, other.root);
	}	
}

/**
 * Private helper function for the constructor. Recursively builds
 * the tree according to the specification of the constructor.
 * @param img reference to the original input image.
 * @param ul upper left point of current node's rectangle.
 * @param lr lower right point of current node's rectangle.
 */
Node* QTree::BuildNode(const PNG& img, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {

	int nodeWidth = lr.first - ul.first;
	int nodeHeight = lr.second - ul.second;

	int splitW = ul.first + (nodeWidth)/2;
	int splitH = ul.second + (nodeHeight)/2;

	Node* NW = nullptr;
	Node* NE = nullptr;
	Node* SW = nullptr;
	Node* SE = nullptr; 

	if ((nodeWidth == 0) && (nodeHeight == 0)) {
		return new Node(ul, lr, *img.getPixel(ul.first, ul.second));;
 	} else if (nodeHeight == 0) {
        NW = BuildNode(img, make_pair(ul.first, ul.second), make_pair(splitW, lr.second));
        NE = BuildNode(img, make_pair(splitW + 1, ul.second), make_pair(lr.first, splitH));
    } else if (nodeWidth == 0) {
        NW = BuildNode(img, make_pair(ul.first, ul.second), make_pair(splitW, splitH));
        SW = BuildNode(img, make_pair(ul.first, splitH + 1), make_pair(splitW, lr.second));
    } else {
        NW = BuildNode(img, make_pair(ul.first, ul.second), make_pair(splitW, splitH));
        NE = BuildNode(img, make_pair(splitW + 1, ul.second), make_pair(lr.first, splitH));
        SW = BuildNode(img, make_pair(ul.first, splitH + 1), make_pair(splitW, lr.second));
        SE = BuildNode(img, make_pair(splitW + 1, splitH + 1), make_pair(lr.first, lr.second));
    }

	Node* newNode = new Node(ul, lr, GetAveragePixel(NW, NE, SW, SE));

	newNode -> NW = NW;
	newNode -> NE = NE;
	newNode -> SW = SW;
	newNode -> SE = SE;

	return newNode;
}

/*********************************************************/
/*** IMPLEMENT YOUR OWN PRIVATE MEMBER FUNCTIONS BELOW ***/
/*********************************************************/

RGBAPixel QTree::GetAveragePixel(Node* NW, Node* NE, Node* SW, Node* SE){
		
	int nwArea;
	int neArea;
	int swArea;
	int seArea;
	RGBAPixel nwP;
	RGBAPixel neP;
	RGBAPixel swP;
	RGBAPixel seP;

	if (NW != nullptr) {
		nwArea = ((NW -> lowRight.first) - (NW -> upLeft.first) + 1) * ((NW -> lowRight.second) - (NW -> upLeft.second) + 1);
		nwP = NW -> avg;
	} else {
		nwArea = 0;
	}

	if(NE != nullptr) {
		neArea = ((NE -> lowRight.first) - (NE -> upLeft.first) + 1) * ((NE -> lowRight.second) - (NE -> upLeft.second) + 1);
		neP = NE -> avg;
	} else {
		neArea = 0;
	}
	
	if(SW != nullptr) {
		swArea = ((SW -> lowRight.first) - (SW -> upLeft.first) + 1) * ((SW -> lowRight.second) - (SW -> upLeft.second) + 1);
		swP = SW -> avg;
	} else {
		swArea = 0;
	}
	
	if(SE != nullptr) {
		seArea = ((SE -> lowRight.first) - (SE -> upLeft.first) + 1) * ((SE -> lowRight.second) - (SE -> upLeft.second) + 1);
		seP = SE -> avg;
	} else {
		seArea = 0;
	}
	
	int totalArea = nwArea + neArea + swArea + seArea;

	double redAvg = (nwP.r * nwArea + neP.r * neArea + swP.r * swArea + seP.r * seArea)/totalArea;
	double greenAvg = (nwP.g * nwArea + neP.g * neArea + swP.g * swArea + seP.g * seArea)/totalArea;
	double blueAvg = (nwP.b * nwArea + neP.b * neArea + swP.b * swArea + seP.b * seArea)/totalArea;
	double aAvg = (nwP.a * nwArea + neP.a * neArea + swP.a * swArea + seP.a * seArea)/totalArea;
	RGBAPixel newP(redAvg, greenAvg, blueAvg, aAvg);
	return newP;

}

void QTree::Render(Node* subroot, unsigned int scale, PNG &img) const {
	if (subroot == nullptr) {
		return;
	}	

	if (subroot -> NW == nullptr && 
	subroot -> NE == nullptr && 
	subroot -> SW == nullptr && 
	subroot -> SE == nullptr) {
		int nodeWidth = subroot -> lowRight.first - subroot -> upLeft.first;
		int nodeHeight = subroot -> lowRight.second - subroot -> upLeft.second;
		RGBAPixel nodeP = subroot -> avg;

		for (int x = 0; x <= nodeWidth; x++) {
			for (int y = 0; y <= nodeHeight; y++){
				for (int xx = 0; xx < scale; xx++) {
					for(int yy = 0; yy < scale; yy++){
						RGBAPixel* imgP = img.getPixel((scale*subroot -> upLeft.first) + x + xx, (scale*subroot -> upLeft.second) + y + yy);
						imgP -> r = nodeP.r;
						imgP -> g = nodeP.g;
						imgP -> b = nodeP.b;
						imgP -> a = nodeP.a;
					}
				}
			}
		}
		
	} else {
		Render(subroot -> NW, scale, img);
		Render(subroot -> NE, scale, img);
		Render(subroot -> SW, scale, img);
		Render(subroot -> SE, scale, img);
	}
	
}

void QTree::RotateCCW(Node* &subroot) {
	if (subroot == nullptr) {
		return;
	}

	pair<unsigned int, unsigned int> ul; 
    pair<unsigned int, unsigned int> lr;

	if (subroot -> NW != nullptr) {
		ul = subroot -> NW -> upLeft;
		lr = subroot -> NW -> lowRight;
		unsigned int tempu = ul.second;
		unsigned int templ = lr.second;
		ul.second = height - lr.first - 1;
		lr.second = height - ul.first - 1;
		ul.first = tempu;
		lr.first = templ;
		subroot -> NW -> upLeft = ul;
		subroot -> NW -> lowRight = lr;

	}

	if (subroot -> NE != nullptr) {
		ul = subroot -> NE -> upLeft;
		lr = subroot -> NE -> lowRight;
		unsigned int tempu = ul.second;
		unsigned int templ = lr.second;
		ul.second = height - lr.first - 1;
		lr.second = height - ul.first - 1;
		ul.first = tempu;
		lr.first = templ;
		subroot -> NE -> upLeft = ul;
		subroot -> NE -> lowRight = lr;
	}

	if (subroot -> SW != nullptr) {
		ul = subroot -> SW -> upLeft;
		lr = subroot -> SW -> lowRight;
		unsigned int tempu = ul.second;
		unsigned int templ = lr.second;
		ul.second = height - lr.first - 1;
		lr.second = height - ul.first - 1;
		ul.first = tempu;
		lr.first = templ;
		subroot -> SW -> upLeft = ul;
		subroot -> SW -> lowRight = lr;
	}

	if (subroot -> SE != nullptr) {
		ul = subroot -> SE -> upLeft;
		lr = subroot -> SE -> lowRight;
		unsigned int tempu = ul.second;
		unsigned int templ = lr.second;
		ul.second = height - lr.first - 1;
		lr.second = height - ul.first - 1;
		ul.first = tempu;
		lr.first = templ;
		subroot -> SE -> upLeft = ul;
		subroot -> SE -> lowRight = lr;
	}


	Node* nwTemp = subroot -> NW;
	Node* seTemp = subroot -> SE;
	subroot -> NW = subroot -> NE;
	subroot -> SE = subroot -> SW;
	subroot -> NE = seTemp;
	subroot -> SW = nwTemp;

	
	RotateCCW(subroot -> NW);
	RotateCCW(subroot -> NE);
	RotateCCW(subroot -> SW);
	RotateCCW(subroot -> SE);

}


void QTree::FlipHorizontal(Node* &subroot) {

	if (subroot == nullptr) {
		return;
	}
	
	pair<unsigned int, unsigned int> ul; 
    pair<unsigned int, unsigned int> lr;

	if (subroot -> NW != nullptr) {
		ul = subroot -> NW -> upLeft;
		lr = subroot -> NW -> lowRight;
		unsigned int temp = lr.first;
		lr.first = width - ul.first - 1;
		ul.first = width - temp - 1;
		subroot -> NW -> upLeft = ul;
		subroot -> NW -> lowRight = lr;

	}

	if (subroot -> NE != nullptr) {
		ul = subroot -> NE -> upLeft;
		lr = subroot -> NE -> lowRight;
		unsigned int temp = lr.first;
		lr.first = width - ul.first - 1;
		ul.first = width - temp - 1;	
		subroot -> NE -> upLeft = ul;
		subroot -> NE -> lowRight = lr;
	}

	if (subroot -> SW != nullptr) {
		ul = subroot -> SW -> upLeft;
		lr = subroot -> SW -> lowRight;
		unsigned int temp = lr.first;
		lr.first = width - ul.first - 1;
		ul.first = width - temp - 1;
		subroot -> SW -> upLeft = ul;
		subroot -> SW -> lowRight = lr;
	}

	if (subroot -> SE != nullptr) {
		ul = subroot -> SE -> upLeft;
		lr = subroot -> SE -> lowRight;
		unsigned int temp = lr.first;
		lr.first = width - ul.first - 1;
		ul.first = width - temp - 1;
		subroot -> SE -> upLeft = ul;
		subroot -> SE -> lowRight = lr;
	}

	Node* nwTemp = subroot -> NW;
	subroot -> NW = subroot -> NE;
	subroot -> NE = nwTemp;
	Node* swTemp = subroot -> SW;
	subroot -> SW = subroot -> SE;
	subroot -> SE = swTemp;
	
	FlipHorizontal(subroot -> NW);
	FlipHorizontal(subroot -> NE);
	FlipHorizontal(subroot -> SW);
	FlipHorizontal(subroot -> SE);
}


void QTree::CopyNodes(Node* &subroot, Node* other) {
	if (other == nullptr) {
		subroot = nullptr;
	} else {
		subroot = new Node(other->upLeft, other->lowRight, other->avg);
		CopyNodes(subroot->NW, other->NW);
		CopyNodes(subroot->NE, other->NE);
		CopyNodes(subroot->SW, other->SW);
		CopyNodes(subroot->SE, other->SE);
	}
}

void QTree::Clear(Node* &subroot) {
	if (subroot != nullptr) {
		Clear(subroot->NW);
		Clear(subroot->NE);
		Clear(subroot->SW);
		Clear(subroot->SE);

		delete subroot;
		subroot = nullptr;
	}
}


void QTree::Prune(Node* &subroot, double tolerance) {

	if (subroot == nullptr) {
		return;
	}

	RGBAPixel nodeP = subroot -> avg;

	if (ValidPrune(subroot->NW, nodeP, tolerance) &&
		ValidPrune(subroot->NE, nodeP, tolerance) &&
		ValidPrune(subroot->SW, nodeP, tolerance) &&
		ValidPrune(subroot->SE, nodeP, tolerance)) {
		Clear(subroot -> NW);
		Clear(subroot -> NE);
		Clear(subroot -> SW);
		Clear(subroot -> SE);
	} else {
		Prune(subroot -> NW, tolerance);
		Prune(subroot -> NE, tolerance);
		Prune(subroot -> SW, tolerance);
		Prune(subroot -> SE, tolerance);
	}

	
}


bool QTree::ValidPrune(Node* subroot, RGBAPixel nodeP, double tolerance) {
	if (subroot == nullptr) {
		return true;
	}

	if (subroot -> NW == nullptr && 
	subroot -> NE == nullptr && 
	subroot -> SW == nullptr && 
	subroot -> SE == nullptr) {
		//Check whether that leaf is less than tolerance
		return nodeP.distanceTo(subroot -> avg) <= tolerance;
	} else {
		return ValidPrune(subroot->NW, nodeP, tolerance) && 
		ValidPrune(subroot->NE, nodeP, tolerance) && 
		ValidPrune(subroot->SW, nodeP, tolerance) && 
		ValidPrune(subroot->SE, nodeP, tolerance);
	}
}

