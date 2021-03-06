#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "fsm.h"

static fsm_bool_t
fsm_default_input_matching_fn(char *transition_key,unsigned int size,char *user_data,unsigned int user_data_len,unsigned int *length_read){
    if(size <= user_data_len){
        if(memcmp(transition_key, user_data, size))
            return FSM_FALSE;
        *length_read = size;
        return FSM_TRUE;
    }
    *length_read = 0;
    return FSM_FALSE;
}

fsm_t *create_new_fsm(const char *fsm_name){

    fsm_t * fsm = calloc(1, sizeof(fsm_t));
    strncpy(fsm->fsm_name, fsm_name, MAX_FSM_NAME_SIZE - 1);
    fsm->fsm_name[MAX_FSM_NAME_SIZE - 1] = '\0';
    fsm_register_input_matching_fn_cb(fsm, 
        fsm_default_input_matching_fn);
    return fsm;
}

void
set_fsm_initial_state(fsm_t *fsm, state_t *state){
    
    assert(!fsm->initial_state);
    fsm->initial_state = state;
}

state_t *
create_new_state(char *state_name,
                 fsm_bool_t is_final){

    assert(state_name);
    
    state_t *state = calloc(1, sizeof(state_t));
    
    strncpy(state->state_name, state_name, MAX_STATE_NAME_SIZE -1);
    state->state_name[MAX_STATE_NAME_SIZE -1] = '\0';

    state->is_final = is_final;
    return state;
}

tt_entry_t *
get_next_empty_tt_entry(tt_t *trans_table){

    tt_entry_t *tt_entry_ptr = NULL;
    
    assert(trans_table);

    FSM_ITERATE_TRANS_TABLE_BEGIN(trans_table, tt_entry_ptr){

    } FSM_ITERATE_TRANS_TABLE_END(trans_table, tt_entry_ptr);

    if(is_tt_entry_empty(tt_entry_ptr) == FSM_TRUE)
        return tt_entry_ptr;

    return NULL;
}


tt_entry_t *
create_and_insert_new_tt_entry(tt_t *trans_table,
        char *transition_key,
        unsigned int sizeof_key,
        output_fn outp_fn,
        state_t *next_state){

    assert(sizeof_key < MAX_TRANSITION_KEY_SIZE);

    tt_entry_t *tt_entry_ptr = get_next_empty_tt_entry(trans_table);
    
    if(!tt_entry_ptr){
        printf("FATAL : Transition Table is Full\n");
        return NULL;
    }

    memcpy(tt_entry_ptr->transition_key, transition_key, sizeof_key);
    tt_entry_ptr->transition_key[sizeof_key] = '\0';
    tt_entry_ptr->transition_key_size = sizeof_key;
    tt_entry_ptr->outp_fn = outp_fn;
    tt_entry_ptr->next_state = next_state;
    return tt_entry_ptr;
}




static fsm_bool_t
fsm_evaluate_transition_entry_match(fsm_t *fsm, tt_entry_t *tt_entry, char *input_buffer,unsigned int input_buffer_len,
                                        unsigned int *length_read) {

   unsigned int i = 0;
   fsm_bool_t is_tt_entry_cb_present = FSM_FALSE;
   fsm_bool_t res = FSM_FALSE;
      
   if(!input_buffer)
       return FSM_TRUE;

   is_tt_entry_cb_present = tt_entry->input_matching_fn_cb[0] ? FSM_TRUE : FSM_FALSE;

   if(is_tt_entry_cb_present){

        for(; i < MAX_TT_ENTRY_CALLBACKS; i++){

            if(!tt_entry->input_matching_fn_cb[i]){
                return FSM_FALSE;
            }

            if((tt_entry->input_matching_fn_cb[i])(
                    NULL, 0, input_buffer, input_buffer_len, length_read)){
                return FSM_TRUE;
            }
            
            *length_read = 0;
        }
        return FSM_FALSE;
   }

   res = fsm->input_matching_fn_cb(tt_entry->transition_key,tt_entry->transition_key_size,input_buffer, input_buffer_len, length_read);

    if(res == FSM_TRUE){
        *length_read = tt_entry->transition_key_size;
    }

   return res;
}

static state_t *
fsm_apply_transition(fsm_t *fsm, state_t *state, char *input_buffer,unsigned int size,unsigned int *length_read, 
                            fsm_output_buff_t *output_buffer){

   tt_entry_t *tt_entry = NULL;
   state_t *next_state = NULL;

   assert(size);

   FSM_ITERATE_TRANS_TABLE_BEGIN((&state->state_trans_table),tt_entry){

        if((tt_entry->transition_key_size <= size) &&fsm_evaluate_transition_entry_match(fsm,tt_entry, input_buffer,size,length_read)){
            
            next_state = tt_entry->next_state;
             
            if(tt_entry->outp_fn){
                tt_entry->outp_fn(state, next_state, 
                            input_buffer, 
                            tt_entry->transition_key_size,
                            output_buffer);
            }

            return next_state;
        }

   }FSM_ITERATE_TRANS_TABLE_END(&state->state_trans_table,tt_entry);

   return NULL;
}


fsm_error_t
execute_fsm(fsm_t *fsm, char *input_buffer,unsigned int size,fsm_output_buff_t *output_buffer,fsm_bool_t *fsm_result){            


   state_t *initial_state = fsm->initial_state;
   assert(initial_state);

   state_t *current_state = initial_state;
   state_t *next_state = NULL;

   fsm->input_buffer_cursor = 0;
   unsigned int length_read = 0;
   unsigned int input_buffer_len = 0;
   char *buffer_to_parse;

   if(fsm_result){
       *fsm_result = FSM_FALSE;
   }

   if(input_buffer && size){
        buffer_to_parse = input_buffer;
        input_buffer_len = size;
   }
   else{
        buffer_to_parse = fsm->input_buffer;
        input_buffer_len = fsm->input_buffer_size;
   }

   if(!output_buffer){
        output_buffer = &fsm->fsm_output_buff;
   }
   
   init_fsm_output_buffer(output_buffer);
   
   while(fsm->input_buffer_cursor < MAX_INP_BUFFER_LEN){
   
        length_read = 0;
         
        next_state = fsm_apply_transition(fsm, current_state, buffer_to_parse + fsm->input_buffer_cursor,
                        (input_buffer_len - fsm->input_buffer_cursor),&length_read, output_buffer);
        
        if(!next_state){
            return FSM_NO_TRANSITION;
        }

        if(length_read){
            fsm->input_buffer_cursor += length_read;
            current_state = next_state;
            if(fsm->input_buffer_cursor == input_buffer_len)
                break;
            continue;
        }
        break;
   }
 
   if(fsm_result){ 
       *fsm_result = current_state->is_final; 
   }

   return FSM_NO_ERROR;
}

void
fsm_register_input_matching_fn_cb(fsm_t *fsm, 
        input_matching_fn input_matching_fn_cb){
    
    fsm->input_matching_fn_cb = input_matching_fn_cb;
}

void
init_fsm_output_buffer(fsm_output_buff_t *fsm_output_buff){
    
    memset(fsm_output_buff->output_buffer, 0, MAX_OUP_BUFFER_LEN);
    fsm_output_buff->curr_pos = 0;
}
