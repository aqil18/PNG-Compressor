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
PNG RenderNode(PNG render, Node* toRender);
