/*
  File: sync.c
  Date: 10/6/2014
  Author: Mitchell Goff
*/



/* =============================== *

  			 Includes

 * =============================== */

#include <stdlib.h>

#include "../core/list.h"
#include "../process/process.h"
#include "sync.h"





/* =============================== *

  			   Data

 * =============================== */

LinkedListNode resource_head = linkedListNode(resource_head);
unsigned long max_resource_id = 0;





/* =============================== *

  		   Implementation

 * =============================== */

/*
  Create a new resource and add it the the resource list
*/

Resource* getResourceWithID(int id, enum ResourceType type) {
	Resource *current;
	forEachElement(current, &resource_head, node) {
		if (current->id == id) {
			return current->type == type ? current : 0;
		}
	}

	return 0;
}




/*
  Sleep on the mutex's waitqueue until the mutex becomes unlocked
*/

Resource* createResourceWithType(enum ResourceType type) {
	Resource *resource = (Resource *) malloc(sizeof(Resource));
	if (!resource) return 0;

	resource->id = ++max_resource_id;
	resource->type = type;
	addLastNode(&resource->node, &resource_head);

	return resource;
}
