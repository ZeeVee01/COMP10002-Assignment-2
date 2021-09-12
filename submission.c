/* Program to evaluate candidate routines for Robotic Process Automation.

  Skeleton program written by Artem Polyvyanyy, artem.polyvyanyy@unimelb.edu.au,
  September 2020, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.

  Student Authorship Declaration:

  (1) I certify that except for the code provided in the initial skeleton
  file, the  program contained in this submission is completely my own
  individual work, except where explicitly noted by further comments that
  provide details otherwise.  I understand that work that has been developed
  by another student, or by me in collaboration with other students, or by
  non-students as a result of request, solicitation, or payment, may not be
  submitted for assessment in this subject.  I understand that submitting for
  assessment work developed by or in collaboration with other students or
  non-students constitutes Academic Misconduct, and may be penalized by mark
  deductions, or by other penalties determined via the University of
  Melbourne Academic Honesty Policy, as described at
  https://academicintegrity.unimelb.edu.au.

  (2) I also certify that I have not provided a copy of this work in either
  softcopy or hardcopy or any other form to any other student, and nor will I
  do so until after the marks are released. I understand that providing my
  work to other students, regardless of my intention or any undertakings made
  to me by that other student, is also Academic Misconduct.

  (3) I further understand that providing a copy of the assignment
  specification to any form of code authoring or assignment tutoring service,
  or drawing the attention of others to such services and code that may have
  been made available via such a service, may be regarded as Student General
  Misconduct (interfering with the teaching activities of the University
  and/or inciting others to commit Academic Misconduct).  I understand that
  an allegation of Student General Misconduct may arise regardless of whether
  or not I personally make use of such solutions or sought benefit from such
  actions.

   Signed by: [Paul Hutchins 1160468]
   Dated:     [21/10/2020]

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

/* #define's -----------------------------------------------------------------*/

#define ASIZE 26
#define LOWER 97 // ASCII value to get array location of lowercase letters
#define UPPER 65 // ASCII value to get array location of uppercase letters
#define NULVAL 2 // NUL value in precon and effect state

/* type definitions ----------------------------------------------------------*/

// state (values of the 26 Boolean variables)
typedef unsigned char state_t[ASIZE];

// action
typedef struct action action_t;
struct action {
    char name;        // action name
    state_t precon;   // precondition
    state_t effect;   // effect
};

// step in a trace
typedef struct step step_t; 
struct step {
    action_t *action; // pointer to an action performed at this step
    step_t   *next;   // pointer to the next step in this trace
    state_t current;  // Stores current value in step for printing trace    
    int valid;        // Boolean value of if struct is valid at step in trace
};

// trace (implemented as a linked list)
typedef struct {
    step_t *head;     // pointer to the step in the head of the trace
    step_t *tail;     // pointer to the step in the tail of the trace
    int length;       // Length of trace
} trace_t;

/* My type definitions -------------------------------------------------------*/

// stzero - output struct for info needed from stage zero
typedef struct {
    action_t (*aa_ptr)[ASIZE]; // Pointer to action array
    trace_t *trace;            // pointer to trace
} stzero_t;

// routine - output struct for routine info
typedef struct {
    char *routine_array; // pointer to routine array
    int routine_length; // length of routine
    int first_char; // first_char for purpose of exiting loop
} routine_t;

// sub-routine - output struct for sub-routine info
typedef struct {
    int sub_length; // length of sub-routine
    char *seq_ptr;  // pointer to sub-routine array
    int found;      // Bool for wether subroutine was found
} sub_routine_t;

/* function prototypes -------------------------------------------------------*/
trace_t* make_empty_trace(void);
trace_t* insert_at_tail(trace_t*, action_t*);
void free_trace(trace_t*);

/* my function prototypes ----------------------------------------------------*/
stzero_t stage_zero();
void get_initial(state_t initial_state);
void action_create(action_t (*p)[ASIZE]);
action_t action_clear(action_t current);
trace_t* create_trace(state_t initial_state, action_t (*aa_ptr)[ASIZE]);
void print_trace(trace_t* trace);
routine_t stage_one(stzero_t input, int stage);
routine_t read_routine(int format);
sub_routine_t routines(state_t routine, stzero_t input, int index, int stage);
void routine_effect(state_t output, routine_t routine, action_t (*aa_ptr)[]);
void print_state(state_t state);
char ascii_convert_lower(char c);
char ascii_convert_upper(char c);
char mygetchar();

/* where it all happens ------------------------------------------------------*/

int
main(int argc, char *argv[]) {
    routine_t routine;
    routine.first_char = 0;
    routine.routine_array = NULL;
    int stage;
    stzero_t output; // stage zero output data storage
    output = stage_zero();    // do stage zero

    if (output.trace->tail->valid == 0){
        // do nothing to skip stage 1 and 2 if invalid trace
    }
    else {
        // if # found stage 1 present, print header and do stage one
        if (mygetchar() == '#') {
            stage = 1;
            printf("==STAGE 1===============================\n"); // Header
            routine = stage_one(output, stage);
        }

        /* has routine.first_char as it makes an easy way to check if we can 
        exit stage 1 and otherwise we have issues with starting stage 2 */
        if (routine.first_char == '#') {
            stage = 2;
            printf("==STAGE 2===============================\n"); // Header
            stage_one(output, stage);
            printf("==THE END===============================\n"); // Footer
        }
    }

    free_trace(output.trace); // Free memory before finishing

    return EXIT_SUCCESS;        // we are done !!! algorithms are fun!!!
}

/* function definitions ------------------------------------------------------*/

// Adapted version of the make_empty_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t
*make_empty_trace(void) {
    trace_t *R;
    R = (trace_t*)malloc(sizeof(*R));
    assert(R!=NULL);
    R->head = R->tail = NULL;
    return R;
}

// Adapted version of the insert_at_foot function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t 
*insert_at_tail(trace_t* R, action_t* addr) {
    step_t *new;
    new = (step_t*)malloc(sizeof(*new));
    assert(R!=NULL && new!=NULL);
    new->action = addr;
    new->next = NULL;
    if (R->tail==NULL) { /* this is the first insertion into the trace */
        R->head = R->tail = new; 
    } else {
        R->tail->next = new;
        R->tail = new;
    }
    return R;
}

// Adapted version of the free_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
void
free_trace(trace_t* R) {
    step_t *curr, *prev;
    assert(R!=NULL);
    curr = R->head;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(R);
}

/* my function definitions ---------------------------------------------------*/

/* takes input file and stores initial_state, actions, and trace.
   Prints all neccesary outputs and returns stage 0 struct */
stzero_t stage_zero() {
    stzero_t output; // output struct
    state_t initial_state = {0};
    trace_t* trace;

    printf("==STAGE 0===============================\n"); // Header

    /* Stores input values to initial_state */
    get_initial(initial_state);

    static action_t action_array[ASIZE]; // Creates action_t array
    action_t (*aa_ptr)[ASIZE] = &action_array; // aa_ptr = ptr to action_array

    /* stores actions in pointer array locations and prints # of actions */
    action_create(aa_ptr);

    /* Creates linked list trace and prints Length of Trace and valididty */
    trace = create_trace(initial_state, aa_ptr);

    /* print headers */
    printf("----------------------------------------\n");
    printf("  abcdefghijklmnopqrstuvwxyz\n");

    /* print initial_state */
    printf("> ");
    print_state(initial_state);
    printf("\n");
    
    print_trace(trace); // prints valid values in trace

    // Stores values to output
    output.aa_ptr = aa_ptr;
    output.trace = trace;
    return output;
}

/* goes through input file and obtains intial state */
void get_initial(state_t initial_state) {
    char c;
    int i;

    // stores intial state as given by file in intial_state
    while(1){
        // read one char
        c = mygetchar();
        // exit on #
        if (c == '#') {
            break;
        }

        // obtain int value of letter, store truth value in inital state array
        i = ascii_convert_lower(c);
        initial_state[i] = 1;
    }

    mygetchar(); // Buffer mygetchar() for \n character
} 

/* takes input file and interprets / stores all actions in array */
void action_create(action_t (*p)[]) {
    action_t current;
    char c;
    int i, index, col_count = 0, actions = 0;

    /* sets all values of current to 0 */
    current = action_clear(current);

    while (1) {
        // read one char
        c = mygetchar();
        // exit on #
        if (c == '#') {
            break;
        }

        index = ascii_convert_lower(c); // convert char to int 1-26

        // if char is : increment col_count to adjust for different data
        if (c == ':') {
            col_count++;
        }

        // if char is \n, reset colcount for new action and store prev action
        else if (c == '\n') {
            col_count = 0;
            actions += 1; // Increments count for actions

            index = ascii_convert_upper(current.name);
            
            // stores current action in location specified by pointer array
            (*p)[index].name = current.name;
            for (i=0; i<ASIZE; i++) {
                (*p)[index].precon[i] = current.precon[i];
                (*p)[index].effect[i] = current.effect[i];
            }

            // Clears current for next action
            current = action_clear(current);
        }

        // True precondition
        else if (col_count == 0) {
            current.precon[index] = 1;
        }

        // False precondition
        else if (col_count == 1) {
            current.precon[index] = 0;
        }

        // action variable
        else if (col_count == 2) {
            current.name = c;
        }

        // True Effect
        else if (col_count == 3) {
            current.effect[index] = 1;
        }

        // False Effect
        else if (col_count == 4) {
            current.effect[index] = 0;
        }
    }

    mygetchar(); // Buffer for \n character

    printf("Number of distinct actions: %d\n", actions);
}

/* Clears current action_t struct by setting all values to NULVAL / 2 */
action_t action_clear(action_t current){
    int i;
    current.name = 0;
    for (i=0; i<ASIZE; i++) {
        current.precon[i] = NULVAL;
        current.effect[i] = NULVAL;
    }
    return current;
}

/* creates the trace in a linked list, returns pointer to head*/
trace_t* create_trace(state_t initial_state, action_t (*aa_ptr)[]) {
    trace_t* trace; // Pointer to head of trace
    state_t current_trace;
    action_t* struct_ptr;
    char c, cur_pre, cur_eff; // Storage for precon and effect
    int index;
    int input_length = 0, valid = 1; // counter for input and bool for validity
    int i; // Loop variable

    /* set all values on current_state to = initial_state */
    for (i=0; i<ASIZE; i++) {
        current_trace[i] = initial_state[i];
    }

    trace = make_empty_trace(); // Make new empty trace

    while (1) {

        c = mygetchar(); // Read one char, c = current step in trace

        // Exit on '\n'
        if (c == '\n') {
            break;
        }

        input_length += 1;

        // Converts trace char to pointer to equivilent action
        index = ascii_convert_upper(c);
        struct_ptr = &(*aa_ptr)[index];

        // Inserts step_t at tail of trace containing pointer to action
        trace = insert_at_tail(trace, struct_ptr);

        /* Using action pointer obtains precon and effect values to make needed
           changes to step_t, by iterating through*/
        for (i=0; i<ASIZE; i++) {

            // Stores current precon and effect values in cur_pre and cur_eff
            cur_pre = trace->tail->action->precon[i];
            cur_eff = trace->tail->action->effect[i];

            trace->tail->valid = 1; // Sets valid to 1 by default

            /* if previous state does not meet precon conditions or trace is
               already invalid sets valid to 0 inside step_t and function */
            if ( ((cur_pre != NULVAL) && (current_trace[i] != cur_pre))
                 || (valid == 0) ){
                trace->tail->valid = 0;
                valid = 0;
            }

            // if effect condition is non NULVAL makes needed changes
            if ( (cur_eff != NULVAL) ){
                current_trace[i] = cur_eff;
            }

            // stores state after changes are made for future comparisons
            trace->tail->current[i] = current_trace[i];
        }
    }

    // Prints input trace length
    trace->length = input_length;
    printf("Length of the input trace: %d\n", input_length);

    // Prints validity of trace
    printf("Trace status: ");
    if (valid) {
        printf("valid\n");
    }
    else {
        printf("invalid\n");
    }

    return trace; // Returns pointer to head of trace
}

/* Prints valid values in trace */
void print_trace(trace_t* trace) {
    step_t* next_val = NULL; // Storage for pointer to next step
    int first = 1; // Bool for handling first item in trace

    while (1) {

        // If trace empty/invalid pointer or first item in trace invalid - break
        if ((trace->head == NULL) || (trace->head->valid == 0)) {
            break;
        }

        // Handling exemption for first item in trace
        else if (first == 1) {
            printf("%c ", trace->head->action->name); // Prints action
            print_state(trace->head->current); // Prints state
            printf("\n");
            next_val = trace->head->next; // changes pointer to next_val
            first = 0; // Stops exemption for first item
        }

        // If end of trace, or next action invalid - break
        else if ((next_val == NULL) || (next_val->valid == 0)) {
            break;
        }

        // Else print trace
        else {
            printf("%c ", next_val->action->name); // Prints action
            print_state(next_val->current); // Prints state
            printf("\n");
            next_val = next_val->next; // Changes pointer to next_val
        }
    }
}

/* Call for stage one and two - implemented in same function */
routine_t stage_one(stzero_t input, int stage) {
    int format = 0; // Stores values for different print formating
    int head_index = 0;
    int i;
    state_t routine_output;
    routine_t routine; // struct for storing routine and info
    sub_routine_t sub_routine;

    sub_routine.found = 0; // initialize so no garbage values in clang

    mygetchar(); // buffer getchar() for /n

    // Loops through for each routine in stage
    while (1) {
 
        // Read current routine and store in routine
        routine = read_routine(format);
        format = 1; // Set format so delimitter prints

        // if no routine present break
        if ((routine.first_char == '#') || (routine.first_char == '\n') || 
                (routine.first_char < 0)) {
            break;
        }

        // Store routine conditons in routine_output
        routine_effect(routine_output, routine, input.aa_ptr);

        // While head_index is not outside trace
        while (head_index < input.trace->length) {

            // Check for routines at index head_index for either stage 1 or 2
            if (stage == 1) {
                sub_routine = routines(routine_output, input, head_index,stage);
            }
            else if (stage == 2) {
                sub_routine = routines(routine_output, input, head_index,stage);
            }

            // If subroutine found
            if (sub_routine.found) {
                
                // print index of subroutine
                printf("%5d: ", head_index);

                // print subroutine
                for (i=0; i<sub_routine.sub_length; i++) {
                    printf("%c", sub_routine.seq_ptr[i]);
                }
                printf("\n");
                
                // Increment head to end of routine
                head_index += (sub_routine.sub_length - 1);
            }

            // Increment head to next step
            head_index++;
        }

        // reset head_index
        head_index = 0;

    }

    // return first char, so we know wether or not stage 2 present
    return routine;
}

/* reads routine from input, prints headers and returns routine and length */
routine_t read_routine(int format) {
    int first = 0; // Bool for storing if first action in routine
    int i; // Loop variable
    char c; // mygetchar() container
    char *ret = NULL; // Pointer to char array
    size_t alloc_size = 4; // Arbitrary initial size
    size_t str_size = 0; // bytes out of alloc_size used
    routine_t routine; // output storage for routine


    ret = (char *)malloc(alloc_size);

     while (1) { 

        // Read one char
        c = mygetchar(); 

        // store first char - used later to determine when stage finishes
        if (first == 0) {
            routine.first_char = c;
        }  

        // if end of stage return routine
        if ((c == '#') || (c < 0)) {
            free(ret);
            return routine;
        }

        // If routine present handles printing header and storing routine
        if (c != '\n') {

            // If first char in candidate routine
            if (first == 0) {

                // if format == 1, i.e not first routine, print delimiter line
                if (format == 1) { 
                printf("----------------------------------------\n");
                }

                // Print header
                printf("Candidate routine: ");
                first = 1; // changes first bool so no longer prints header

                // New routine so free pointer and make new array
                free(ret); // Frees memory allocated to char array
                alloc_size = 4; // resets alloc_size
                ret = (char *)malloc(alloc_size); // remakes char array
                str_size = 0; // resets string size
            }
        }
        
        // If string size is greater then alloc_size, realloc to double size
        if (str_size >= alloc_size) {
            alloc_size *= 2;
            ret = realloc(ret, alloc_size);
        }

        // Store char
        ret[str_size] = c;

        // If end of routine
        if (c == '\n') {

            // iterate through routine array and print
            for (i=0; i<str_size; i += sizeof(char)) {
                printf("%c", ret[i]);
            }
            
            // Acounts for end of file printing new line
            if (str_size > 0) {
                printf("\n");
            }

            // Store values in routine and return
            routine.routine_array = ret;
            routine.routine_length = str_size;
            return routine;
        }

        // Increment str_size
        str_size++;

    }
}

/* takes routine, input trace, head_index and stage and does either the basic
routine or advanced routine checks based on stage and returns sub_routine_t 
struct containing sub_routine length, bool if found and char array of routine */
sub_routine_t routines(state_t routine, stzero_t input, int index, int stage) {
    char *seq_ptr = NULL; // pointer to current sub sequence
    int i = 0; // loop variable
    int valid = 1; // Boolean if current sub sequence is valid
    state_t test_state; // test_state to store current cumulative sub-routine
    size_t alloc_size = 4; // intial abritrarty alloc size
    size_t seq_length = 0; // counter for seq_length
    step_t *head = input.trace->head; // pointer to firss action in trace
    sub_routine_t sub_routine; // output variable for returning sub-routine
    state_t precons; // Storage for preconditions before sub_routine


    /* loops through precon state, if head_index is 0, intial_state used as 
    preconditons, else uses head pointer of trace action before current */
    for (i=0; i<index; i++) {
        head = head->next;
    }

    // sets start values for current and precon
    for (i=0; i<ASIZE; i++) {
        test_state[i] = NULVAL;
        precons[i] = NULVAL;
    }

    // Creates char array of alloc_size, seq_ptr points to array
    seq_ptr = (char *)malloc(alloc_size);

    while (1) {
        // If head pointer is NULL, inidcating end of trace, break
        if (head == NULL) {
            break;
        }

        // iterate through test_state and precons to update values
        for (i=0; i<ASIZE; i++) {

            // if value in action.effect is non NUL update test_state
            if (head->action->effect[i] != NULVAL) {
                test_state[i] = head->action->effect[i];
            }
            
            // if precons == NULVAL assign action precon
            if (precons[i] == NULVAL) {
                precons[i] = head->action->precon[i];
            }
        }


        // if seq_length is greater then alloc_size double alloc size
        if (seq_length >= alloc_size) {
            alloc_size *= 2;
            seq_ptr = realloc(seq_ptr, alloc_size);
        }

        // Store action name
        seq_ptr[seq_length] = head->action->name;

        // Increment seq_length
        seq_length++;

        // iterate through routine, precons and test_state to test validity
        for (i=0; i<ASIZE; i++) {
            if (stage == 1) {
                // if routine val is NUL & current val != Nul, no routine found
                if ((routine[i] == NULVAL) && (test_state[i] != NULVAL)) {
                    sub_routine.found = 0;
                    return sub_routine;
                }
            
                // else if routine val != current val, routine may still be found
                else if (routine[i] != test_state[i]) {
                    valid = 0;
                }
            }

            if (stage == 2) {
                // If routine value NUL and test_state != NUL or != precon, fail
                    if ((routine[i] == NULVAL) && (test_state[i] != NULVAL)
                        && (test_state[i] != precons[i])){
                        valid = 0;
                    }

                // if routine value non NULL and test_state != routine fail
                    if ((routine[i] != NULVAL) && (test_state[i] != routine[i])) {
                        valid = 0;
                    }
            }
            
        }

        // if sub routine found
        if (valid == 1) {
            sub_routine.found = 1; // Set true
            sub_routine.sub_length = seq_length; // Set sub rotuine length
            sub_routine.seq_ptr = seq_ptr; // set sub routine pointer
            return sub_routine; // return struct
        }

        // Resets Valid
        valid = 1;

        // Sets head to next action in trace
        head = head->next;

    }

    // else no sub routine found
    sub_routine.found = 0;
    return sub_routine;

}

/* takes routine and iterates through to get the effect of routine in output */
void routine_effect(state_t output, routine_t routine, action_t (*aa_ptr)[]) {
    int i, j, d; // Counter and index storage variables
    char c; // char for storing action
    action_t *action_ptr; // pointer to action

    // Sets all vals in output to NULVAL
    for (i=0; i<ASIZE; i++) {
        output[i] = NULVAL;
    }

    // Iterates through routine
    for (i=0; i<routine.routine_length; i++) {

        // Gets action
        c = routine.routine_array[i];

        // Converts action to array index
        d = ascii_convert_upper(c);

        // Points to action in action array
        action_ptr = &(*aa_ptr)[d];

        // iterates through length of action.effect
        for (j=0; j<ASIZE; j++) {

            // If action.effect is not NULVAL sets output = action.effect
            if (action_ptr->effect[j] != NULVAL) {
                output[j] = action_ptr->effect[j];
            }
        }
    }
}

/* prints all values in state, used to stop repetition of code */
void print_state(state_t state) {
    int i;

    for (i=0; i<ASIZE; i++) {
        printf("%d", state[i]);
    }
}

/* simple helper function to convert lowercase letters into int 0-25 */
char ascii_convert_lower(char c) {
    char i;
    i = c - LOWER;
    return i;
}

/* simple helper function to convert uppercase letters into int 0-25 */
char ascii_convert_upper(char c) {
    char i;
    i = c - UPPER;
    return i;
}

/* Accounts for errors with '\r' character */
char mygetchar() {
    char c;
    while ( (c=getchar()) == '\r' ) {
    }
    return c;
}

/******************************************************************************/
/*            _      _____  ____  _____  _____ _______ _    _ __  __  _____   */
/*      /\   | |    / ____|/ __ \|  __ \|_   _|__   __| |  | |  \/  |/ ____|  */
/*     /  \  | |   | |  __| |  | | |__) | | |    | |  | |__| | \  / | (___    */
/*    / /\ \ | |   | | |_ | |  | |  _  /  | |    | |  |  __  | |\/| |\___ \   */
/*   / ____ \| |___| |__| | |__| | | \ \ _| |_   | |  | |  | | |  | |____) |  */
/*  /_/    \_\______\_____|\____/|_|  \_\_____|  |_|  |_|  |_|_|  |_|_____/   */
/*                          _____  ______   ______ _    _ _   _               */
/*                    /\   |  __ \|  ____| |  ____| |  | | \ | |              */
/*                   /  \  | |__) | |__    | |__  | |  | |  \| |              */
/*                  / /\ \ |  _  /|  __|   |  __| | |  | | . ` |              */
/*                 / ____ \| | \ \| |____  | |    | |__| | |\  |              */
/*                /_/    \_\_|  \_\______| |_|     \____/|_| \_|              */
/*                                                                            */
/******************************************************************************/ 