/*
 *
 * KIAVC pathfinding utilities.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_PATHFINDING_H
#define __KIAVC_PATHFINDING_H

#include <stdbool.h>

#include "list.h"

typedef struct kiavc_pathfinding_point {
	/* Coordinates */
	int x, y;
} kiavc_pathfinding_point;

typedef struct kiavc_pathfinding_walkbox {
	/* Name of the walkbox, if any */
	char *name;
	/* Coordinates of the walkbox rectangle */
	kiavc_pathfinding_point p1, p2;
	/* Scale factor to apply to actors, if needed */
	float scale;
	/* Speed modifier factor to apply to actors, if needed */
	float speed;
	/* Whether the walkbox is disabled or not */
	bool disabled;
} kiavc_pathfinding_walkbox;

typedef struct kiavc_pathfinding_node {
	/* Point this node corresponds to */
	kiavc_pathfinding_point point;
	/* Walkboxes this node connects */
	kiavc_pathfinding_walkbox *w1, *w2;
	/* Other nodes this is connected to */
	kiavc_list *neighbours;
	/* A* info for this node */
	Uint16 f, g, h;
	/* Parent node */
	struct kiavc_pathfinding_node *parent;
} kiavc_pathfinding_node;

typedef struct kiavc_pathfinding_context {
	/* List of walkboxes in this context */
	kiavc_list *walkboxes;
	/* List of nodes for pathfinding purposes */
	kiavc_list *nodes;
} kiavc_pathfinding_context;

/* Helper to create a new point instance */
kiavc_pathfinding_point *kiavc_pathfinding_point_create(int x, int y);
/* Helper to destroy a point instance */
void kiavc_pathfinding_point_destroy(kiavc_pathfinding_point *point);

/* Helper to create a new walkbox */
kiavc_pathfinding_walkbox *kiavc_pathfinding_walkbox_create(const char *name,
	int x1, int y1, int x2, int y2, float scale, float speed, bool disabled);
/* Helper to check if two walkboxes overlap */
bool kiavc_pathfinding_walkboxes_overlap(kiavc_pathfinding_walkbox *w1, kiavc_pathfinding_walkbox *w2);
/* Helper to get the rectangle intersection of two walkboxes */
int kiavc_pathfinding_walkboxes_interception(kiavc_pathfinding_walkbox *w1, kiavc_pathfinding_walkbox *w2,
	kiavc_pathfinding_point *p1, kiavc_pathfinding_point *p2);
/* Helper to check if a point is in a specific walkbox */
bool kiavc_pathfinding_walkbox_contains(kiavc_pathfinding_walkbox *walkbox, kiavc_pathfinding_point *point);
/* Helper to destroy a walkbox instance */
void kiavc_pathfinding_walkbox_destroy(kiavc_pathfinding_walkbox *walkbox);

/* Helper to create a new node instance */
kiavc_pathfinding_node *kiavc_pathfinding_node_create(kiavc_pathfinding_point *point,
	kiavc_pathfinding_walkbox *w1, kiavc_pathfinding_walkbox *w2);
/* Helper to destroy a node instance */
void kiavc_pathfinding_node_destroy(kiavc_pathfinding_node *node);

/* Helper to create a new pathfinding context */
kiavc_pathfinding_context *kiavc_pathfinding_context_create(void);
/* Helper to recalculate a pathfinding context */
int kiavc_pathfinding_context_recalculate(kiavc_pathfinding_context *pathfinding);
/* Helper to find the walkbox a point is in */
kiavc_pathfinding_walkbox *kiavc_pathfinding_context_find_walkbox(kiavc_pathfinding_context *pathfinding,
	kiavc_pathfinding_point *point);
/* Helper to find the point closest to any walkbox from a reference */
int kiavc_pathfinding_context_find_closest(kiavc_pathfinding_context *pathfinding,
	kiavc_pathfinding_point *point, kiavc_pathfinding_point *closest);
/* Helper to find a path as a series of points to walk to */
kiavc_list *kiavc_pathfinding_context_find_path(kiavc_pathfinding_context *pathfinding,
	kiavc_pathfinding_point *from, kiavc_pathfinding_point *to);
/* Helper to destroy a pathfinding context instance */
void kiavc_pathfinding_context_destroy(kiavc_pathfinding_context *pathfinding);

#endif
