#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){

    node_t* new_node = (node_t*)malloc(sizeof(node_t));
	
	//FILL IN MISSING CODE
	new_node->parent = n;
	new_node->depth = n->depth + 1;
	new_node->move = action;
	copy_state(&(new_node->state), &(n->state));
	new_node->state.cursor = *selected_peg;
    execute_move_t( &(new_node->state), &(new_node->state.cursor), action );
	
	return new_node;

}

/**
 * Find a solution path as per algorithm description in the handout
 */

void find_solution( state_t* init_state  ){

	HashTable table;
	int remainingPegs, nodes = 0;
	node_t** nodeArr = (node_t**)malloc(budget * sizeof(node_t));

	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	//Add the initial node
	node_t* n = create_init_node( init_state );
	
	//FILL IN THE GRAPH ALGORITHM
	stack_push(n);
	remainingPegs = num_pegs(&(n->state));
	ht_insert(&table, &(n->state), n);
	while(!is_stack_empty()){
		position_s new_pos;
		n = stack_top();
		stack_pop();
		expanded_nodes += 1;
		
		/*Keep track of all the nodes being popped
		from the stack to free later.
		*/
		nodeArr[nodes] = n;
		nodes++;
		
		//Check to see if a better solution is found
		if(num_pegs(&(n->state)) < remainingPegs){
			save_solution(n);
			remainingPegs = num_pegs(&(n->state));
		}
		
		/*Check for a possible legal action in the state
		of the current node.
		*/
		for(int i = 0; i < SIZE; i++){
			new_pos.x = i;
			for(int j = 0; j < SIZE; j++){
				new_pos.y = j;
				for(int move = 0; move < 4; move++){
					if(can_apply(&(n->state), &new_pos, move)){
						
						//Generate a child node if a legal action is found
						node_t *newNode;
						newNode = applyAction(n, &new_pos, move);
						generated_nodes += 1;
						
						/*If the new node's state is has solved
						the Peg Solitaire*/
						if(won(&(newNode->state))){
							save_solution(newNode);
							remainingPegs = num_pegs(&(newNode->state));
							ht_destroy(&table);
							free_stack();
							for(int j=0;j<nodes;j++){
								free(nodeArr[j]);
							}
							free(nodeArr);
							free(newNode);
							return;
						}
						
						/*Check and remove any duplicates to make the algorithm 
						more efficient 
						*/
						if(ht_contains(&table, &(newNode->state)) == 
						   HT_NOT_FOUND){
							ht_insert(&table, &(newNode->state), newNode);
							stack_push(newNode);
						}
						else{
							free(newNode);
						}
					}
				}
			}
		}
		
		/* Stop the algorithm when budget is exceeded. 
		*/
		if(expanded_nodes >= budget){
			ht_destroy(&table);
			free_stack();
			for(int j=0;j<nodes;j++){
				free(nodeArr[j]);
			}
			free(nodeArr);
			return;
		}
	}
}