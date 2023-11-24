/**
 * @file qtree-private.h
 * @description student declaration of private QTree functions
 *              CPSC 221 PA3
 *
 *              SUBMIT THIS FILE.
 * 
 *				Simply declare your function prototypes here.
 *              No other scaffolding is necessary.
 */

// begin your declarations below

RGBAPixel GetAveragePixel(Node* NW, Node* NE, Node* SW, Node* SE);
void Render(Node* subroot, unsigned int scale, PNG &img) const;
void FlipHorizontal(Node* &subroot);
void RotateCCW(Node* &subroot);
void CopyNodes(Node* &subroot, Node* other);
void Clear(Node* &subroot);
