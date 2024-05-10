// COMP3511 Spring 2024
// PA3: Page Replacement Algorithms
//
// Please use the print-related helper functions, instead of using your own printf function calls
// The print-related helper functions are clearly defined in the skeleton code
// The grader TA will probably use an autograder to grade this PA
//
// Your name:
// Your ITSC email:           @connect.ust.hk
//
// Declaration:
//
// I declare that I am not involved in plagiarism
// I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

// ===
// Region: Header files
// Note: Necessary header files are included, do not include extra header files
// ===
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ===
// Region: Constants
// ===

#define UNFILLED_FRAME -1
#define MAX_QUEUE_SIZE 10
#define MAX_FRAMES_AVAILABLE 10
#define MAX_REFERENCE_STRING 30

#define ALGORITHM_FIFO "FIFO"
#define ALGORITHM_OPT "OPT"
#define ALGORITHM_LRU "LRU"


// Keywords (to be used when parsing the input)
#define KEYWORD_ALGORITHM "algorithm"
#define KEYWORD_FRAMES_AVAILABLE "frames_available"
#define KEYWORD_REFERENCE_STRING_LENGTH "reference_string_length"
#define KEYWORD_REFERENCE_STRING "reference_string"



// Assume that we only need to support 2 types of space characters:
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"

// ===
// Region: Global variables:
// For simplicity, let's make everything static without any dyanmic memory allocation
// In other words, we don't need to use malloc()/free()
// It will save you lots of time to debug if everything is static
// ===
char algorithm[10];
int reference_string[MAX_REFERENCE_STRING];
int reference_string_length;
int frames_available;
int frames[MAX_FRAMES_AVAILABLE];

// Helper function: Check whether the line is a blank line (for input parsing)
int is_blank(char *line)
{
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch))
            return 0;
        ch++;
    }
    return 1;
}
// Helper function: Check whether the input line should be skipped
int is_skip(char *line)
{
    if (is_blank(line))
        return 1;
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}
// Helper: parse_tokens function
void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

// Helper: parse the input file
void parse_input()
{
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2];                                 // buffer for 2 tokens
    char *reference_string_tokens[MAX_REFERENCE_STRING]; // buffer for the reference string
    int numTokens = 0, n = 0, i = 0;
    char equal_plus_spaces_delimiters[5] = "";

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters, SPACE_CHARS);

    while ((nread = getline(&line, &len, fp)) != -1)
    {
        if (is_skip(line) == 0)
        {
            line = strtok(line, "\n");
            if (strstr(line, KEYWORD_ALGORITHM))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    strcpy(algorithm, two_tokens[1]);
                }
            }
            else if (strstr(line, KEYWORD_FRAMES_AVAILABLE))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &frames_available);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING_LENGTH))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &reference_string_length);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING))
            {
                parse_tokens(two_tokens, line, &numTokens, "=");
                if (numTokens == 2)
                {
                    parse_tokens(reference_string_tokens, two_tokens[1], &n, SPACE_CHARS);
                    for (i = 0; i < n; i++)
                    {
                        sscanf(reference_string_tokens[i], "%d", &reference_string[i]);
                    }
                }
            }
        }
    }
}
// Helper: Display the parsed values
void print_parsed_values()
{
    int i;
    printf("%s = %s\n", KEYWORD_ALGORITHM, algorithm);
    printf("%s = %d\n", KEYWORD_FRAMES_AVAILABLE, frames_available);
    printf("%s = %d\n", KEYWORD_REFERENCE_STRING_LENGTH, reference_string_length);
    printf("%s = ", KEYWORD_REFERENCE_STRING);
    for (i = 0; i < reference_string_length; i++)
        printf("%d ", reference_string[i]);
    printf("\n");
}

// Useful string template used in printf()
// We will use diff program to auto-grade this project assignment
// Please use the following templates in printf to avoid formatting errors
//
// Example:
//
//   printf(template_total_page_fault, 0)    # Total Page Fault: 0 is printed on the screen
//   printf(template_no_page_fault, 0)       # 0: No Page Fault is printed on the screen

const char template_total_page_fault[] = "Total Page Fault: %d\n";
const char template_no_page_fault[] = "%d: No Page Fault\n";

// Helper function:
// This function is useful for printing the fault frames in this format:
// current_frame: f0 f1 ...
//
// For example: the following 4 lines can use this helper function to print
//
// 7: 7
// 0: 7 0
// 1: 7 0 1
// 2: 2 0 1
//
// For the non-fault frames, you should use template_no_page_fault (see above)
//
void display_fault_frame(int current_frame)
{
    int j;
    printf("%d: ", current_frame);
    for (j = 0; j < frames_available; j++)
    {
        if (frames[j] != UNFILLED_FRAME)
            printf("%d ", frames[j]);
        else
            printf("  ");
    }
    printf("\n");
}

// Helper function: initialize the frames
void init_frames()
{
    int i;
    for (i = 0; i < frames_available; i++)
        frames[i] = UNFILLED_FRAME;
}
int match (int s){
    for (int i = 0; i < frames_available; i++){
        if(frames[i] == s)
            return 1;
    }
    return -1;
}
void FIFO_algorithm()
{
    // TODO: Implement FIFO alogrithm here
    int totalFault = 0;
    int CurS = 0;
    int CurF = 0;
    int victimFrame = 0;
    while(CurS < reference_string_length){
        int s = reference_string[CurS];
            if(frames[CurF] == UNFILLED_FRAME && match(s) == -1){
                frames[CurF] = s;
                CurF++;
                CurS++;
                totalFault++;
                display_fault_frame(s);
                continue;
            }
            for(int i = 0 ; i < frames_available ; i++){// checking match
                if(frames[i] == s){
                    printf(template_no_page_fault, s);
                    CurS++;
                    break;
                }
                if(i == frames_available - 1){ // No Match
                    frames[victimFrame++] = s;
                    CurS++;
                    totalFault++;
                    display_fault_frame(s);
                }   
            }
            if (victimFrame == frames_available)
                victimFrame = 0;            
        }
    printf(template_total_page_fault, totalFault);
}

void OPT_algorithm()
{
    // TODO: Implement OPT replacement here
    int CurF = 0 ,totalFault = 0;
    int victimFrame = -1;
    int checkingFrames[MAX_FRAMES_AVAILABLE];
    int checkingFrames_available = frames_available;
    for (int CurS = 0 ; CurS < reference_string_length ; CurS++){
        for(int i = 0 ; i < frames_available; i++)
            checkingFrames[i] = frames[i];
        int s = reference_string[CurS];
        if(frames[CurF] == UNFILLED_FRAME && match(s) == -1){ // initialize when still empty
            frames[CurF] = s;
            CurF++;
            totalFault++;
            display_fault_frame(s);
            continue;
        }
         for(int i = 0 ; i < frames_available ; i++){// checking match
            if(frames[i] == s){
                printf(template_no_page_fault, s);
                break; // skip to line next CurS loop
            }
            if(i == frames_available - 1){ // no match
                victimFrame = -1;
                if(CurS + 1 != reference_string_length){
                    for(int j = CurS + 1 ; j < reference_string_length ; j++){
                        for(int k = 0 ; k < checkingFrames_available; k++){
                            if(reference_string[j] == checkingFrames[k]){
                                checkingFrames[k] = UNFILLED_FRAME;
                                victimFrame = k;
                            }
                        }
                        for(int j = 0 ; j < checkingFrames_available; j++){
                            if(checkingFrames[j] != -1){
                                victimFrame = j;
                                break;
                            }
                        }
                    }
                frames[victimFrame] = s;
                }else frames[0] = s;
                totalFault++;
                display_fault_frame(s);
            }
        }
    }
        printf(template_total_page_fault, totalFault);
}
void LRU_algorithm()
{
    // TODO: Implement LRU replacement here
    //counter Implement
    int totalFault = 0,CurS = 0,CurF = 0,victimFrame = -1;
    int checkingFrames[MAX_FRAMES_AVAILABLE];
    int checkingFrames_available = frames_available;
    while(CurS < reference_string_length){
        for(int i = 0 ; i < frames_available; i++)
            checkingFrames[i] = frames[i];
        int s = reference_string[CurS];
        if(frames[CurF] == UNFILLED_FRAME && match(s) == -1){ // NO MATCH & EMPTY LEFT
            frames[CurF] = s;
            CurF++;
            CurS++;
            totalFault++;
            display_fault_frame(s);
            continue;
        }
        for(int i = 0 ; i < frames_available ; i++){// checking match
            if(frames[i] == s){
                printf(template_no_page_fault, s);
                CurS++;
                break; // skip to line next CurS loop
            }
            //remember to handle edge outbound case of 0
           if( i == frames_available - 1){ // No Match Ready to choose victimFrame
                victimFrame = -1;
                for(int j = CurS - 1; j >= 0 ; j--){
                    for(int k = 0 ; k < checkingFrames_available; k++){
                        if(checkingFrames[k] == reference_string[j]){
                            checkingFrames[k] = UNFILLED_FRAME;
                            victimFrame = k;
                        }
                    }
                }
                frames[victimFrame] = s;
                totalFault++;
                CurS++;
                display_fault_frame(s);
           }
        }
    }
   printf(template_total_page_fault, totalFault);
}


int main()
{
    parse_input();              
    print_parsed_values();      
    init_frames();              
    if (strcmp(algorithm, ALGORITHM_FIFO) == 0)
    {
        FIFO_algorithm();
    }
    else if (strcmp(algorithm, ALGORITHM_OPT) == 0)
    {
        OPT_algorithm();
    }
    else if (strcmp(algorithm, ALGORITHM_LRU) == 0)
    {
        LRU_algorithm();
    }

    return 0;
}