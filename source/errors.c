#include <clay/errors.h>
#include <clay/macros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static inline char *clay_strdup(const char *source) {
  char *result;
  CLAY_strdup(result, source);
  return result;
}

char *clay_error_message_text(int status_result) {
  switch (status_result) {
    case CLAY_ERROR_BETA_NOT_FOUND:
      return clay_strdup("the beta vector was not found");
    case CLAY_ERROR_NOT_BETA_LOOP:
      return clay_strdup("the beta is not a loop");
    case CLAY_ERROR_NOT_BETA_STMT:
      return clay_strdup("the beta is not a statement");
    case CLAY_ERROR_REORDER_ARRAY_TOO_SMALL:
      return clay_strdup("the order array is too small");
    case CLAY_ERROR_REORDER_ARRAY_SIZE:
      return clay_strdup("the order array is too small or too big");
    case CLAY_ERROR_DEPTH_OVERFLOW:
      return clay_strdup("depth overflow");
    case CLAY_ERROR_WRONG_COEFF:
      return clay_strdup("wrong coefficient");
    case CLAY_ERROR_BETA_EMPTY:
      return clay_strdup("the beta vector is empty");
    case CLAY_ERROR_BETA_NOT_IN_A_LOOP:
      return clay_strdup("the beta need to be in a loop");
    case CLAY_ERROR_WRONG_BLOCK_SIZE:
      return clay_strdup("block value is incorrect");
    case CLAY_ERROR_WRONG_FACTOR:
      return clay_strdup("wrong factor");
    case CLAY_ERROR_DEPTH_OUTER:
      return clay_strdup("the depth is not 'outer'");
    case CLAY_ERROR_VECTOR_EMPTY:
      return clay_strdup("the vector is empty");
    case CLAY_ERROR_IDENT_NAME_NOT_FOUND:
      return clay_strdup("the iterator name was not found");
    case CLAY_ERROR_IDENT_STMT_NOT_FOUND:
      return clay_strdup("the statement was not found");
    case CLAY_ERROR_INEQU:
      return clay_strdup("the inequality seems "
                         "to be wrong");
    case CLAY_ERROR_VECTOR:
      return clay_strdup("the vector seems to be wrong");
    case CLAY_ERROR_REORDER_OVERFLOW_VALUE:
      return clay_strdup("there is an overflow value on the reorder array");
    case CLAY_ERROR_CANT_PRIVATIZE:
      return clay_strdup("privatization failed");
    case CLAY_ERROR_ARRAYS_EXT_EMPTY:
      return clay_strdup("arrays extensions is empty");
    case CLAY_ERROR_ID_EXISTS:
      return clay_strdup("the id already exists");
    case CLAY_ERROR_UNK_VAR:
      return clay_strdup("error on the variable "
                         "(for now only a-z variables are accepted)");
    case CLAY_ERROR_VAR_NULL:
      return clay_strdup("the variable was not defined");
    case CLAY_ERROR_CANT_CALL_SUBFUNC:
      return clay_strdup("sorry you can't at the moment call "
                         "a function in an other function. Please use a temp "
                         "variable.");
    case CLAY_ERROR_ARRAY_NOT_FOUND:
      return clay_strdup("the array was not found");
    case CLAY_ERROR_ARRAY_NOT_FOUND_IN_THE_BETA:
      return clay_strdup(
             "the array was not found in the given beta");
    case CLAY_ERROR_BETAS_NOT_SAME_DIMS:
      return clay_strdup("the betas don't have the same dimensions");
    case CLAY_ERROR_BETAS_NOT_SAME_DOMAIN:
      return clay_strdup("the betas don't have the same domain");
    case CLAY_ERROR_ONE_HAS_EXTBODY:
      return clay_strdup("one of the statement has an extbody "
                         "but the other one");
    case CLAY_ERROR_WRONG_BETA:
      return clay_strdup("transformation is not applicable to the "
                         "given beta");
    case CLAY_SUCCESS:
      return NULL;
    default:
    {
      const char *prefix = "unknown error";
      char *message;
      unsigned long size = strlen(prefix) + 32;
      CLAY_malloc(message, char *, size);
      snprintf(message, size, "%s %d", prefix, status_result);
      return message;
    }
  }
}

