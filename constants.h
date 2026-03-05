#define SRC_FILETYPE "as"
#define POST_PROCESS_FILETYPE "am"
#define ENT_FILETYPE "ent"
#define EXT_FILETYPE "ext"
#define OBJ_FILETYPE "ob"
#define MAX_LINE_LEN 81 /* 80 + 1 for '\0' */
#define MAX_MACRO_LEN 31
#define MAX_OPERATION_LEN 5
#define MAX_LABEL_LEN 31
#define MEM_START_POINT 100
#define NUM_OF_REGS 8
#define NUM_OF_OPS 16
#define NUM_OF_TYPES 4
#define MEMORY_SIZE 1024
#define WORD_SIZE 12 /* word size refers to the size of a single memory cell which is 12 */
#define OPERAND_SIZE 3
#define OPERATION_SIZE 4
#define REGISTRY_SIZE 5
#define MAX_INST_NUM_VAL 511
#define MIN_INST_NUM_VAL -512
#define MAX_DATA_NUM_VAL 2047    /* the max value for a number represented by 12 bits */
#define MIN_DATA_NUM_VAL -2048   /* the min value for a number represented by 12 bits */
#define MAX_ADDRESS_LEN 4
#define ARE_SIZE 2
#define EXTERN_BINCODE "000000000001"
#define RELOC_CODE 2
#define BASE64_LEN 2
#define EOF_MARK 255

/* effective for all pointer types */
#define CHECK_MEM_ALLOC(ptr) \
if (ptr == NULL) {               \
    fprintf(stderr,"Memory allocation failed. Terminating.\n"); \
    exit(1); \
    }
typedef enum {DATA, STRING, INSTRUCTION, ENTRY, EXTERN, INVALID_TAG} Directive_tag;
typedef enum {NONE, IMMEDIATE, DIRECT = 3, DIRECT_REG = 5, INVALID_TYPE} Assignment_type;
typedef enum {FALSE, TRUE} boolean; /*  001010 000000        000000 010100 */
