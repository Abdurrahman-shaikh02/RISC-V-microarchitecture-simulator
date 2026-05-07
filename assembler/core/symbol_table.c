#include "assembler.h"
// lets make a linked list that we will linear search everytime... i know bad choice but i really dont want to do the hash table implementation
symbol_node * get_symbol(char * s){
	// NULL for failure
	if(!head) return NULL;
	symbol_node * temp = head;
	while(temp != NULL){
		if(!strcmp(temp->name, s)){
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

int get_address(char * s){
	// -1 for error
	symbol_node * node = get_symbol(s);
	if(!node) return -1;

	return node->address;
}

int add_symbol(char * s ,int address){
	// -1 for error
	if(strlen(s) > 255) return -1;

	if(get_symbol(s)) return -1;


	symbol_node * t = (symbol_node * )malloc(sizeof(symbol_node));

	t->next = head;
	head = t;
	
	strcpy(t->name, s);
	t->address = address;

	return 1;

}

void free_symbol_table(){
	symbol_node * temp = head;

	while(head){
		temp = head;
		head = head->next;
		free(temp);
	}
	return;
}


