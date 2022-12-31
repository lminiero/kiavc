/*
 *
 * KIAVC pathfinding utilities.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL.h>

#include "pathfinding.h"

#define KIAVC_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define KIAVC_MIN(x, y) (((x) < (y)) ? (x) : (y))

/* Helper to calculate a squared distance */
static float kiavc_pathfinding_distance(int x1, int y1, int x2, int y2) {
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y2 - y2));
}

/* Helper function to perform the A* algorithm */
static kiavc_list *kiavc_pathfinding_astar(kiavc_list *from, kiavc_list *to);
/* Helper function to find a smoother path using line of sight */
static kiavc_list *kiavc_pathfinding_smoothen(kiavc_pathfinding_context *pathfinding, kiavc_list *path);
/* Helper function that uses the Bresenham algorithm to check if two points have line of sight */
static bool kiavc_pathfinding_lineofsight(kiavc_pathfinding_context *pathfinding,
	kiavc_pathfinding_point *p1, kiavc_pathfinding_point *p2);

/* Helper to create a new point instance */
kiavc_pathfinding_point *kiavc_pathfinding_point_create(int x, int y) {
	kiavc_pathfinding_point *point = SDL_malloc(sizeof(kiavc_pathfinding_point));
	point->x = x;
	point->y = y;
	return point;
}

/* Helper to destroy a point instance */
void kiavc_pathfinding_point_destroy(kiavc_pathfinding_point *point) {
	if(point)
		SDL_free(point);
}

/* Helper to create a new walkbox */
kiavc_pathfinding_walkbox *kiavc_pathfinding_walkbox_create(const char *name,
		int x1, int y1, int x2, int y2, float scale, float speed, bool disabled) {
	kiavc_pathfinding_walkbox *walkbox = SDL_malloc(sizeof(kiavc_pathfinding_walkbox));
	walkbox->name = name ? SDL_strdup(name) : NULL;
	walkbox->p1.x = x1;
	walkbox->p1.y = y1;
	walkbox->p2.x = x2;
	walkbox->p2.y = y2;
	walkbox->scale = scale;
	walkbox->speed = speed;
	walkbox->disabled = disabled;
	return walkbox;
}

/* Helper to check if two walkboxes overlap */
bool kiavc_pathfinding_walkboxes_overlap(kiavc_pathfinding_walkbox *w1, kiavc_pathfinding_walkbox *w2) {
	if(!w1 || !w2 || w1->disabled || w2->disabled)
		return false;
	return (w1->p1.x <= w2->p2.x && w1->p2.x >= w2->p1.x &&
		w1->p1.y <= w2->p2.y && w1->p2.y >= w2->p1.y);
}

/* Helper to get the rectangle intersection of two walkboxes */
int kiavc_pathfinding_walkboxes_interception(kiavc_pathfinding_walkbox *w1, kiavc_pathfinding_walkbox *w2,
		kiavc_pathfinding_point *p1, kiavc_pathfinding_point *p2) {
	if(!w1 || !w2 || w1->disabled || w2->disabled)
		return -1;
	int x1 = w1->p1.x > w2->p1.x ? w1->p1.x : w2->p1.x;
	int x2 = w1->p2.x < w2->p2.x ? w1->p2.x : w2->p2.x;
	int y1 = w1->p1.y > w2->p1.y ? w1->p1.y : w2->p1.y;
	int y2 = w1->p2.y < w2->p2.y ? w1->p2.y : w2->p2.y;
	if(x1 > x2 || y1 > y2)
		return -1;
	SDL_Log("  -- Interception between '%s' and '%s': [%dx%d] <-> [%dx%d]\n",
		w1->name, w2->name, x1, y1, x2, y2);
	if(p1) {
		p1->x = x1;
		p1->y = y1;
	}
	if(p2) {
		p2->x = x2;
		p2->y = y2;
	}
	return 0;
}

/* Helper to check if a point is in a specific walkbox */
bool kiavc_pathfinding_walkbox_contains(kiavc_pathfinding_walkbox *walkbox, kiavc_pathfinding_point *point) {
	if(!walkbox || walkbox->disabled || !point)
		return false;
	return (point->x >= walkbox->p1.x && point->y >= walkbox->p1.y &&
		point->x <= walkbox->p2.x && point->y <= walkbox->p2.y);
}

/* Helper to destroy a walkbox instance */
void kiavc_pathfinding_walkbox_destroy(kiavc_pathfinding_walkbox *walkbox) {
	if(walkbox) {
		if(walkbox->name)
			SDL_free(walkbox->name);
		SDL_free(walkbox);
	}
}

/* Helper to create a new node instance */
kiavc_pathfinding_node *kiavc_pathfinding_node_create(kiavc_pathfinding_point *point,
		kiavc_pathfinding_walkbox *w1, kiavc_pathfinding_walkbox *w2) {
	if(!point)
		return NULL;
	kiavc_pathfinding_node *node = SDL_calloc(1, sizeof(kiavc_pathfinding_node));
	node->point.x = point->x;
	node->point.y = point->y;
	node->w1 = w1;
	node->w2 = w2;
	return node;
}

/* Helper to destroy a node instance */
void kiavc_pathfinding_node_destroy(kiavc_pathfinding_node *node) {
	if(node) {
		kiavc_list_destroy(node->neighbours);
		SDL_free(node);
	}
}

/* Helper to create a new pathfinding context */
kiavc_pathfinding_context *kiavc_pathfinding_context_create(void) {
	kiavc_pathfinding_context *pathfinding = SDL_calloc(1, sizeof(kiavc_pathfinding_context));
	return pathfinding;
}

/* Helper to recalculate a pathfinding context */
int kiavc_pathfinding_context_recalculate(kiavc_pathfinding_context *pathfinding) {
	if(!pathfinding)
		return -1;
	g_list_free_full(pathfinding->nodes, (GDestroyNotify)kiavc_pathfinding_node_destroy);
	pathfinding->nodes = NULL;
	kiavc_list *temp = pathfinding->walkboxes, *temp2 = NULL;
	kiavc_pathfinding_walkbox *w1 = NULL, *w2 = NULL;
	kiavc_pathfinding_node *n1 = NULL, *n2 = NULL, *n3 = NULL, *n4 = NULL, *nm = NULL;
	kiavc_pathfinding_point p1 = { 0 }, p2 = { 0 }, p3 = { 0 }, p4 = { 0 }, pm = { 0 };
	while(temp) {
		w1 = (kiavc_pathfinding_walkbox *)temp->data;
		temp2 = temp->next;
		while(temp2) {
			w2 = (kiavc_pathfinding_walkbox *)temp2->data;
			bool overlap = kiavc_pathfinding_walkboxes_overlap(w1, w2);
			SDL_Log("Walkboxes '%s' and '%s' %s overlap\n",
				w1->name ? w1->name : "unnamed",
				w2->name ? w2->name : "unnamed",
				overlap ? "do" : "DON'T");
			if(overlap && kiavc_pathfinding_walkboxes_interception(w1, w2, &p1, &p2) == 0) {
				n1 = kiavc_pathfinding_node_create(&p1, w1, w2);
				n2 = kiavc_pathfinding_node_create(&p2, w1, w2);
				pathfinding->nodes = kiavc_list_append(pathfinding->nodes, n1);
				pathfinding->nodes = kiavc_list_append(pathfinding->nodes, n2);
				if(p1.x != p2.x && p1.y != p2.y) {
					/* Add the two other vertices of the rectangle */
					p3.x = p1.x;
					p3.y = p2.y;
					p4.x = p2.x;
					p4.y = p1.y;
					n3 = kiavc_pathfinding_node_create(&p3, w1, w2);
					n4 = kiavc_pathfinding_node_create(&p4, w1, w2);
					pathfinding->nodes = kiavc_list_append(pathfinding->nodes, n3);
					pathfinding->nodes = kiavc_list_append(pathfinding->nodes, n4);
				}
				/* Add nodes at the center of the sides */
				if(p1.x != p2.x) {
					pm.x = (p1.x + p2.x) / 2;
					pm.y = p1.y;
					nm = kiavc_pathfinding_node_create(&pm, w1, w2);
					pathfinding->nodes = kiavc_list_append(pathfinding->nodes, nm);
					if(p1.y != p2.y) {
						pm.x = (p1.x + p2.x) / 2;
						pm.y = p2.y;
						nm = kiavc_pathfinding_node_create(&pm, w1, w2);
						pathfinding->nodes = kiavc_list_append(pathfinding->nodes, nm);
					}
				}
				if(p1.y != p2.y) {
					pm.x = p1.x;
					pm.y = (p1.y + p2.y) / 2;
					nm = kiavc_pathfinding_node_create(&pm, w1, w2);
					pathfinding->nodes = kiavc_list_append(pathfinding->nodes, nm);
					if(p1.x != p2.x) {
						pm.x = p2.x;
						pm.y = (p1.y + p2.y) / 2;
						nm = kiavc_pathfinding_node_create(&pm, w1, w2);
						pathfinding->nodes = kiavc_list_append(pathfinding->nodes, nm);
					}
				}
			}
			temp2 = temp2->next;
		}
		temp = temp->next;
	}
	/* Done */
	return 0;
}
/* Helper to find the point closest to any walkbox from a reference */
int kiavc_pathfinding_context_find_closest(kiavc_pathfinding_context *pathfinding,
		kiavc_pathfinding_point *point, kiavc_pathfinding_point *closest) {
	if(!pathfinding || !point || !closest)
		return -1;
	float distance = 0, min_distance = -1;
	int x = 0, y = 0;
	kiavc_list *temp = pathfinding->walkboxes;
	kiavc_pathfinding_walkbox *w = NULL;
	while(temp) {
		w = (kiavc_pathfinding_walkbox *)temp->data;
		x = KIAVC_MAX(w->p1.x, KIAVC_MIN(w->p2.x, point->x));
		y = KIAVC_MAX(w->p1.y, KIAVC_MIN(w->p2.y, point->y));
		distance = kiavc_pathfinding_distance(point->x, point->y, x, y);
		if(min_distance == -1 || min_distance > distance) {
			min_distance = distance;
			closest->x = x;
			closest->y = y;
		}
		temp = temp->next;
	}
	return 0;
}

/* Helper to find the walkbox a point is in */
kiavc_pathfinding_walkbox *kiavc_pathfinding_context_find_walkbox(kiavc_pathfinding_context *pathfinding,
		kiavc_pathfinding_point *point) {
	if(!pathfinding || !point)
		return NULL;
	kiavc_list *temp = pathfinding->walkboxes;
	kiavc_pathfinding_walkbox *w = NULL;
	while(temp) {
		w = (kiavc_pathfinding_walkbox *)temp->data;
		if(kiavc_pathfinding_walkbox_contains(w, point))
			return w;
		temp = temp->next;
	}
	/* Not found */
	return NULL;
}

/* Helper to find a path as a series of points to walk to */
kiavc_list *kiavc_pathfinding_context_find_path(kiavc_pathfinding_context *pathfinding,
		kiavc_pathfinding_point *from, kiavc_pathfinding_point *to) {
	if(!pathfinding || !pathfinding->nodes || !from || !to)
		return NULL;
	/* TODO */
	kiavc_list *path = NULL;
	kiavc_pathfinding_walkbox *w1 = kiavc_pathfinding_context_find_walkbox(pathfinding, from);
	if(!w1) {
		SDL_Log("Starting from outside of a walkbox?");
	}
	kiavc_pathfinding_walkbox *w2 = kiavc_pathfinding_context_find_walkbox(pathfinding, to);
	if(!w2) {
		/* The target is outside of a walkbox, find the mearest point to one */
		kiavc_pathfinding_point closest = { 0 };
		if(kiavc_pathfinding_context_find_closest(pathfinding, to, &closest) < 0) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Target out of bounds\n");
			return NULL;
		}
		SDL_Log("Changed target point [%d,%d] to [%d,%d]\n",
			to->x, to->y, closest.x, closest.y);
		to->x = closest.x;
		to->y = closest.y;
		w2 = kiavc_pathfinding_context_find_walkbox(pathfinding, to);
	}
	if(w1 == w2) {
		/* Easy enough, it's a direct line */
		SDL_Log("Target is in the same walkbox, path is a direct line\n");
		kiavc_pathfinding_point *p = kiavc_pathfinding_point_create(from->x, from->y);
		path = kiavc_list_append(path, p);
		p = kiavc_pathfinding_point_create(to->x, to->y);
		path = kiavc_list_append(path, p);
	} else {
		/* Actually find path between the two walkboxes */
		SDL_Log("Target is in a different walkbox, calculating path\n");
		/* Let's start by creating a copy of the nodes, and les't add
		 * both the source and target coordinates as nodes there too */
		kiavc_list *temp = pathfinding->nodes, *temp2 = NULL, *nodes = NULL;
		kiavc_pathfinding_node *n1 = NULL, *n2 = NULL;
		n1 = kiavc_pathfinding_node_create(from, w1, NULL);
		nodes = kiavc_list_append(nodes, n1);
		while(temp) {
			n1 = (kiavc_pathfinding_node *)temp->data;
			n2 = kiavc_pathfinding_node_create(&n1->point, n1->w1, n1->w2);
			nodes = kiavc_list_append(nodes, n2);
			temp = temp->next;
		}
		n1 = kiavc_pathfinding_node_create(to, w2, NULL);
		nodes = kiavc_list_append(nodes, n1);
		/* Now let's connect all the nodes */
		temp = nodes;
		while(temp) {
			n1 = (kiavc_pathfinding_node *)temp->data;
			temp2 = temp->next;
			while(temp2) {
				n2 = (kiavc_pathfinding_node *)temp2->data;
				if((n1->w1 && (n1->w1 == n2->w1 || n1->w1 == n2->w2)) || (n1->w2 && (n1->w2 == n2->w1 || n1->w2 == n2->w2))) {
					/* These nodes are connected */
					n1->neighbours = kiavc_list_append(n1->neighbours, n2);
					n2->neighbours = kiavc_list_append(n2->neighbours, n1);
				}
				temp2 = temp2->next;
			}
			temp = temp->next;
		}
		/* Use the A* algorithm to find the path */
		path = kiavc_pathfinding_astar(nodes, g_list_last(nodes));
		if(path) {
			int steps = kiavc_list_size(path);
			SDL_Log("Calculated %d steps to get to the target\n", steps);
			kiavc_pathfinding_point *p = NULL;
			temp = path;
			while(temp) {
				p = (kiavc_pathfinding_point *)temp->data;
				SDL_Log("  -- [%d,%d]\n", p->x, p->y);
				temp = temp->next;
			}
			p = kiavc_pathfinding_point_create(from->x, from->y);
			path = kiavc_list_prepend(path, p);
			/* Use line of sight to see if we can smoothen the path */
			path = kiavc_pathfinding_smoothen(pathfinding, path);
			if(path) {
				int smoothened = kiavc_list_size(path->next);
				if(smoothened < steps) {
					SDL_Log("Shortened to %d steps to get to the target\n", smoothened);
					temp = path->next;
					while(temp) {
						p = (kiavc_pathfinding_point *)temp->data;
						SDL_Log("  -- [%d,%d]\n", p->x, p->y);
						temp = temp->next;
					}
				}
			}
		}
		/* Done */
		g_list_free_full(nodes, (GDestroyNotify)kiavc_pathfinding_node_destroy);
	}
	return path;
}

/* Helper to destroy a pathfinding context instance */
void kiavc_pathfinding_context_destroy(kiavc_pathfinding_context *pathfinding) {
	if(pathfinding) {
		g_list_free_full(pathfinding->walkboxes, (GDestroyNotify)kiavc_pathfinding_walkbox_destroy);
		g_list_free_full(pathfinding->nodes, (GDestroyNotify)kiavc_pathfinding_node_destroy);
		SDL_free(pathfinding);
	}
}

/* Helper to compare nodes when inserting in the priority queue */
static int kiavc_pathfinding_compare(kiavc_pathfinding_node *n1, kiavc_pathfinding_node *n2, void *data) {
	if(!n1 && !n2)
		return 0;
	else if(!n1)
		return -1;
	else if(!n2)
		return 1;
	return n1->f - n2->f;
}

/* Helper function to perform the A* algorithm */
static kiavc_list *kiavc_pathfinding_astar(kiavc_list *from, kiavc_list *to) {
	if(!from || !to)
		return NULL;
	kiavc_list *path = NULL;
	kiavc_pathfinding_node *start = (kiavc_pathfinding_node *)from->data;
	kiavc_pathfinding_node *end = (kiavc_pathfinding_node *)to->data;
	/* Prepare maps to keep track of what we have explored, costs, etc. */
	GHashTable *closed_set = g_hash_table_new(NULL, NULL);
	/* Prepare a priority queue */
	GQueue *open_set = g_queue_new();
	g_queue_insert_sorted(open_set, start, (GCompareDataFunc)kiavc_pathfinding_compare, NULL);
	kiavc_pathfinding_node *current = NULL, *next = NULL;
	kiavc_list *temp = NULL;
	Uint16 g = 0, h = 0, f = 0;
	/* Iterate until we have a path */
	bool found = false;
	while(!g_queue_is_empty(open_set)) {
		current = g_queue_pop_head(open_set);
		g_hash_table_insert(closed_set, current, current);
		if(current == end) {
			/* We're done */
			found = true;
			break;
		}
		temp = current->neighbours;
		while(temp) {
			next = (kiavc_pathfinding_node *)temp->data;
			if(g_hash_table_lookup(closed_set, next)) {
				temp = temp->next;
				continue;
			}
			g = current->g + kiavc_pathfinding_distance(
				current->point.x, current->point.y, next->point.x, next->point.y);
			h = kiavc_pathfinding_distance(next->point.x, next->point.y,
				end->point.x, end->point.y);
			f = g + h;
			if(next->g == 0 || g < next->g) {
				next->g = g;
				next->h = h;
				next->f = f;
			}
			if(!g_queue_find(open_set, next)) {
				g_queue_insert_sorted(open_set, next, (GCompareDataFunc)kiavc_pathfinding_compare, NULL);
				next->parent = current;
			}
			temp = temp->next;
		}
	}
	if(found) {
		/* Prepare list to return */
		while(current && current->parent) {
			path = kiavc_list_prepend(path, kiavc_pathfinding_point_create(current->point.x, current->point.y));
			current = current->parent;
		}
	}
	g_hash_table_destroy(closed_set);
	g_queue_free(open_set);
	return path;
}

/* Helper function to find a smoother path using line of sight */
static kiavc_list *kiavc_pathfinding_smoothen(kiavc_pathfinding_context *pathfinding, kiavc_list *path) {
	if(!pathfinding || !pathfinding->walkboxes || !path)
		return NULL;
	/* If there's only one step, nothing we need to do */
	if(kiavc_list_size(path) == 2)
		return path;
	/* Now that we have a path made of multiple steps, we check if we
	 * can skip some, using line of sight to see if two points can
	 * actually be connected directly even though they're in different
	 * walkboxes, rather than going through the border points */
	kiavc_pathfinding_point *p1 = NULL, *p2 = NULL;
	GList *start = path, *temp = NULL;
	while(kiavc_list_size(start) > 2) {
		p1 = (kiavc_pathfinding_point *)start->data;
		temp = g_list_last(start);
		while(temp != start->next) {
			p2 = (kiavc_pathfinding_point *)temp->data;
			if(kiavc_pathfinding_lineofsight(pathfinding, p1, p2)) {
				/* There is line of sight, get rid of intermediate steps */
				SDL_Log("There's line of sight between [%d,%d] and [%d,%d]\n", p1->x, p1->y, p2->x, p2->y);
				while(start->next != temp) {
					SDL_free(start->next->data);
					path = kiavc_list_remove(path, start->next->data);
				}
				break;
			}
			SDL_Log("There's NO line of sight between [%d,%d] and [%d,%d]\n", p1->x, p1->y, p2->x, p2->y);
			temp = temp->prev;
		}
		start = start->next;
	}
	return path;
}

/* Helper function that uses the Bresenham algorithm to check if two points have line of sight */
static bool kiavc_pathfinding_lineofsight(kiavc_pathfinding_context *pathfinding,
		kiavc_pathfinding_point *p1, kiavc_pathfinding_point *p2) {
	/* Adapted from https://gist.github.com/bert/1085538#file-plot_line-c */
	if(!pathfinding || !pathfinding->walkboxes || !p1 || !p2)
		return false;
	int dx = abs(p2->x - p1->x), sx = p1->x < p2->x ? 1 : -1;
	int dy = -abs(p2->y - p1->y), sy = p1->y < p2->y ? 1 : -1;
	int err = dx + dy, e2 = 0; /* error value e_xy */
	kiavc_pathfinding_point p = { .x = p1->x, .y = p1->y };
	while(true) {
		if(kiavc_pathfinding_context_find_walkbox(pathfinding, &p) == NULL) {
			/* This point in the line is not in any walkbox, which means
			 * we don't have line of sight between the two target points */
			return false;
		}
		if(p.x == p2->x && p.y == p2->y)
			break;
		e2 = 2*err;
		if(e2 >= dy) {
			/* e_xy+e_x > 0 */
			err += dy;
			p.x += sx;
		}
		if(e2 <= dx) {
			/* e_xy+e_y < 0 */
			err += dx;
			p.y += sy;
		}
	}
	/* If we got here, there's line of sight */
	return true;
}
