/*
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 */

/**
 * @file
 * @brief Packing definitions
 */

#include "common.h"

/*
 * Private declarations
 */

/**
 * @addtogroup drizzle_pack_private Private Packing Functions
 * @ingroup drizzle_pack
 * @{
 */

/**
 * Compute hash from password and scramble.
 */
static drizzle_return_t _pack_scramble_hash(drizzle_con_st *con,
                                            uint8_t *buffer);

/** @} */

/*
 * Public definitions
 */

uint8_t *drizzle_pack_length(uint64_t number, uint8_t *ptr)
{
  if (number < 251)
  {
    ptr[0]= (uint8_t)number;
    ptr++;
  }
  else if (number < 65536)
  {
    ptr[0]= 252;
    ptr++;
    drizzle_set_byte2(ptr, number);
    ptr+= 2;
  }
  else if (number < 16777216)
  {
    ptr[0]= 253;
    ptr++;
    drizzle_set_byte3(ptr, number);
    ptr+= 3;
  }
  else
  {
    ptr[0]= 254;
    ptr++;
    drizzle_set_byte8(ptr, number);
    ptr+= 8;
  }

  return ptr;
}

uint64_t drizzle_unpack_length(drizzle_con_st *con, drizzle_return_t *ret_ptr)
{
  uint64_t length;
  uint8_t bytes;

  if (con->buffer_ptr[0] < 251)
  {
    length= (uint64_t)(con->buffer_ptr[0]);
    bytes= 1;
  }
  else if (con->buffer_ptr[0] == 251)
  {
    con->buffer_ptr++;
    con->buffer_size--;
    con->packet_size--;

    *ret_ptr= DRIZZLE_RETURN_NULL_SIZE;
    return 0;
  }
  else if (con->buffer_ptr[0] == 252 && con->buffer_size > 2)
  {
    length= drizzle_get_byte2(con->buffer_ptr + 1);
    bytes= 3;
  }
  else if (con->buffer_ptr[0] == 253 && con->buffer_size > 3)
  {
    length= drizzle_get_byte3(con->buffer_ptr + 1);
    bytes= 4;
  }
  else if (con->buffer_size > 8)
  {
    length= drizzle_get_byte8(con->buffer_ptr + 1);
    bytes= 9;
  }
  else
  {
    *ret_ptr= DRIZZLE_RETURN_IO_WAIT;
    return 0;
  }

  con->buffer_ptr+= bytes;
  con->buffer_size-= bytes;
  con->packet_size-= bytes;

  *ret_ptr= DRIZZLE_RETURN_OK;
  return length;
}

uint8_t *drizzle_pack_string(char *string, uint8_t *ptr)
{
  uint64_t size= strlen(string);

  ptr= drizzle_pack_length(size, ptr);
  if (size > 0)
  {
    memcpy(ptr, string, (size_t)size);
    ptr+= size;
  }

  return ptr;
}

drizzle_return_t drizzle_unpack_string(drizzle_con_st *con, char *buffer,
                                       uint64_t max_length)
{
  drizzle_return_t ret= DRIZZLE_RETURN_OK;
  uint64_t length;

  length= drizzle_unpack_length(con, &ret);
  if (ret != DRIZZLE_RETURN_OK)
  {
    if (ret == DRIZZLE_RETURN_NULL_SIZE)
    {
      drizzle_set_error(con->drizzle, "drizzle_unpack_string",
                        "unexpected NULL length");
    }

    return ret;
  }

  if (length < max_length)
  {
    if (length > 0)
      memcpy(buffer, con->buffer_ptr, (size_t)length);

    buffer[length]= 0;
  }
  else
  {
    memcpy(buffer, con->buffer_ptr, (size_t)(max_length - 1));
    buffer[max_length - 1]= 0;
  }
  
  con->buffer_ptr+= length;
  con->buffer_size-= (size_t)length;
  con->packet_size-= (size_t)length;

  return DRIZZLE_RETURN_OK;
}

uint8_t *drizzle_pack_auth(drizzle_con_st *con, uint8_t *ptr,
                           drizzle_return_t *ret_ptr)
{
  if (con->user[0] != 0)
  {
    memcpy(ptr, con->user, strlen(con->user));
    ptr+= strlen(con->user);
  }

  ptr[0]= 0;
  ptr++;

  if (con->options & DRIZZLE_CON_RAW_SCRAMBLE && con->scramble != NULL)
  {
    ptr[0]= DRIZZLE_MAX_SCRAMBLE_SIZE;
    ptr++;

    memcpy(ptr, con->scramble, DRIZZLE_MAX_SCRAMBLE_SIZE);
    ptr+= DRIZZLE_MAX_SCRAMBLE_SIZE;
  }
  else if (con->password[0] == 0)
  {
    ptr[0]= 0;
    ptr++;
    con->packet_size-= DRIZZLE_MAX_SCRAMBLE_SIZE;
  }
  else
  {
    ptr[0]= DRIZZLE_MAX_SCRAMBLE_SIZE;
    ptr++;

    if (con->options & DRIZZLE_CON_MYSQL)
    {
      *ret_ptr= _pack_scramble_hash(con, ptr);
      if (*ret_ptr != DRIZZLE_RETURN_OK)
        return ptr;
    }
    else
      snprintf((char *)ptr, DRIZZLE_MAX_SCRAMBLE_SIZE, "%s", con->password);

    ptr+= DRIZZLE_MAX_SCRAMBLE_SIZE;
  }

  if (con->db[0] != 0)
  {
    memcpy(ptr, con->db, strlen(con->db));
    ptr+= strlen(con->db);
  }

  ptr[0]= 0;
  ptr++;

  *ret_ptr= DRIZZLE_RETURN_OK;
  return ptr;
}

/*
 * Private definitions
 */

static drizzle_return_t _pack_scramble_hash(drizzle_con_st *con,
                                            uint8_t *buffer)
{
  SHA1_CTX ctx;
  uint8_t hash_tmp1[SHA1_DIGEST_LENGTH];
  uint8_t hash_tmp2[SHA1_DIGEST_LENGTH];
  uint32_t x;

  if (SHA1_DIGEST_LENGTH != DRIZZLE_MAX_SCRAMBLE_SIZE)
  {
    drizzle_set_error(con->drizzle, "_pack_scramble_hash",
                      "SHA1 hash size mismatch:%u:%u", SHA1_DIGEST_LENGTH,
                      DRIZZLE_MAX_SCRAMBLE_SIZE);
    return DRIZZLE_RETURN_INTERNAL_ERROR;
  }

  if (con->scramble == NULL)
  {
    drizzle_set_error(con->drizzle, "_pack_scramble_hash",
                      "no scramble buffer");
    return DRIZZLE_RETURN_NO_SCRAMBLE;
  }

  /* First hash the password. */
  SHA1Init(&ctx);
  SHA1Update(&ctx, (uint8_t *)(con->password), strlen(con->password));
  SHA1Final(hash_tmp1, &ctx);

  /* Second, hash the password hash. */
  SHA1Init(&ctx);
  SHA1Update(&ctx, hash_tmp1, SHA1_DIGEST_LENGTH);
  SHA1Final(hash_tmp2, &ctx);

  /* Third, hash the scramble and the double password hash. */
  SHA1Init(&ctx);
  SHA1Update(&ctx, con->scramble, SHA1_DIGEST_LENGTH);
  SHA1Update(&ctx, hash_tmp2, SHA1_DIGEST_LENGTH);
  SHA1Final(buffer, &ctx);

  /* Fourth, xor the last hash against the first password hash. */
  for (x= 0; x < SHA1_DIGEST_LENGTH; x++)
    buffer[x]= buffer[x] ^ hash_tmp1[x];

  return DRIZZLE_RETURN_OK;
}
