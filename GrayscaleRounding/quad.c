#include <stdlib.h>
#include <math.h>                   /* for lrint */
#include "quad.h"
#include "pgmio.h"
#include <stdio.h>

 //Create a new Quad node, with given tx, ty and width
Quad *newNode(int tx, int ty, int width)
{
	Quad *node = NULL; 
	node = (Quad *)calloc(1, sizeof(Quad));
	if (node == NULL)
	{
		printf("Cannot allocate memory for node.");
		return NULL;
	}
	node->tx = tx;
	node->ty = ty;
	node->width = width;
	for(int i = 0; i < 4; i++) 
		node->children[i] = NULL;
	return node;
}

/* Reconstruct image from quadtree.  Write into the Image record that im points
 * to.  Assume that the square region described by the quadtree fits into the
 * Image.
 *
 * Recall that each Quad node already contains position information; respect it,
 * and you will benefit from it too (a secret for successful recursions).
 */
void saveQuad(Image *im, Quad *q)
{
    if (q == NULL) return;
		
	if (q->children[0] == NULL)
	{	
		int pos = 0;
		for (int i = q->tx; i < q->tx + q->width; i++)
		{
			for(int j = q->ty; j < q->ty + q->width; j++)
			{	
				pos = i + j*(im->sx); 
				im->data[pos] = q->grayscale;
			}
		}
		return;
	}

	for (int i = 0; i < 4; i++) 
		saveQuad(im, q->children[i]);
}


/* Build quadtree from image.  Splitting stops when maxDepth hits 0 or maximum
 * difference in grayscale is <= threshold.  Do this to only the square region
 * at top-left corner (tx, ty) of width w.  Use heap allocation for all Quad
 * nodes you produce.
 */
Quad *formQuadtree(Image *im, int maxDepth, int threshold, int tx, int ty, int w)
{
	int diff = 0, avg = 0, sum = 0, pos = 0;
	int min = im->data[tx + ty*(im->sx)], max = im->data[tx + ty*(im->sx)];
	
	for (int i = tx; i < tx + w; i++)       //find quad sum, min, max to find avg, diff
	{
		for(int j = ty; j < ty + w; j++)
		{
			pos = i + j*(im->sx); 
			sum += im->data[pos];
			
			if (im->data[pos] > max)
				max = im->data[pos];
			
			if (im->data[pos] < min)
				min = im->data[pos];
		}
	}
	avg = lrint((double)sum/(w*w));
	diff = max - min;
	
	Quad *quad = newNode(tx, ty, w);
	if (quad == NULL) return NULL; //base case: memory didnt get allocated for the node
	
	if (diff <= threshold || maxDepth == 0) //base case: diff < threshold or maximum tree depth reached, set grayscale value for this quad to the average and return quad
	{
		quad->grayscale = avg;
		return quad;
	}
	//recursive case: if the difference is larger than threshold and max depth is not reached, subdivide this quad and return it
	quad->children[0] = formQuadtree(im, maxDepth - 1, threshold, tx, ty, w/2);
	quad->children[1] = formQuadtree(im, maxDepth - 1, threshold, tx+w/2, ty, w/2);
	quad->children[2] = formQuadtree(im, maxDepth - 1, threshold, tx, ty+w/2, w/2);
	quad->children[3] = formQuadtree(im, maxDepth - 1, threshold, tx+w/2, ty+w/2, w/2);
	return quad;
}

/* Deallocate all Quad nodes of the given quadtree. Assume that they all came
 * from the heap.
 */
void freeQuadtree(Quad *q)
{
	if (q != NULL)
	{
		for (int i = 0; i < 4; i++)
			freeQuadtree(q->children[i]);
		free(q);
	}
}