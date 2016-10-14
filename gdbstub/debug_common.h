/**
 *  @file debug_common.h
 *
 *  @section LICENSE
 *  Copyright (C) 2009 Kalray
 *  @author Patrice, GERIN patrice.gerin@kalray.eu
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __DEBUG_COMMON_H__
#define __DEBUG_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <unistd.h>
#include <stdio.h>

struct configuration_s;

typedef struct debug_ostream {
  long ref_counter;
  FILE *file;
  char *name;
  struct debug_ostream *prev;
  struct debug_ostream *next;
} debug_ostream_t;

typedef struct {
  int               debug;
  int               warning;

  int               level;
  debug_ostream_t   *ostream;
} debug_attributes_t;

int debug_attributes_ctor( debug_attributes_t *attr, struct configuration_s *cfg);
int debug_attributes_dtor( debug_attributes_t *attr);


/** @} */ /* end of component_common_group */

#ifdef __cplusplus
}
#endif /* __cplusplus */



#define __DEBUG_MESSAGE( _this, fmt, ... ) \
  if( _this->attributes.debug ) fprintf(_this->attributes.ostream->file, "[%-20s] " fmt , _this->name(),  ##__VA_ARGS__ )

#define __ERROR_MESSAGE( _this, fmt, ... ) \
  if( _this->attributes.debug ) fprintf(_this->attributes.ostream->file, "[%-20s] " fmt , _this->name(),  ##__VA_ARGS__ )

#define DEBUG_WARNING( fmt, ... ) \
    fprintf(this->attributes.ostream->file, "[%-20s] " fmt , this->name(),  ##__VA_ARGS__ )

#ifndef NDEBUG
#define DEV_WARNING( fmt, ... ) if( this->attributes.debug ) fprintf(this->attributes.ostream->file, "[%-20s *** DEV WARNING ***] " fmt, this->name(),  ##__VA_ARGS__ )
#else /* NDEBUG */
#define DEV_WARNING( fmt, ... ) 
#endif /* NDEBUG */

#define VERBOSE_INFO( fmt, ... ) if( this->attributes.debug ) fprintf(this->attributes.ostream->file, "[%-20s] " fmt, this->name(),  ##__VA_ARGS__ )

#define MESSAGE_FAILURE( fmt, ... ) fprintf(stderr, "[%-12s] " fmt, get_name(),  ##__VA_ARGS__ )
#define MESSAGE_SUCCESS( fmt, ... ) fprintf(stderr, "[%-12s] " fmt, get_name(),  ##__VA_ARGS__ )

#define DEBUG_LEVEL_INIT    0
#define DEBUG_LEVEL_INFO    1
#define DEBUG_LEVEL_VERB    2
#define DEBUG_LEVEL_FULL    3
#define DEBUG_LEVEL_DEV     4

#ifndef __cplusplus

#define DEBUG_MSG( this, _level, fmt, ... ) \
  if( this->debug_attr.debug && (this->debug_attr.level >= _level) ) fprintf(this->debug_attr.ostream->file, "[%-20s] " fmt , this->attributes.name, ##__VA_ARGS__ )

#define DEBUG_MSG_APPEND( this, _level, fmt, ... ) \
  if( this->debug_attr.debug && (this->debug_attr.level >= _level) ) fprintf(this->debug_attr.ostream->file, fmt ,  ##__VA_ARGS__ )

#define ERROR_MSG( this, fmt, ... ) \
{ \
  if( isatty(fileno(this->debug_attr.ostream->file)) ) { \
    fprintf(this->debug_attr.ostream->file, "\033[1;31m[%-20s] " fmt "\033[0m", this->attributes.name,  ##__VA_ARGS__ ); \
  } else { \
    fprintf(this->debug_attr.ostream->file, "[%-20s] " fmt , this->attributes.name,  ##__VA_ARGS__ ); \
  } \
}

#define STDERR_MSG( fmt, ... ) \
  fprintf(stderr, "%s:%d error: " fmt , __func__, __LINE__, ##__VA_ARGS__ ); 

#define WARNING_MSG( this, fmt, ... ) \
{ \
  if (this->debug_attr.warning  ) {			 \
  if( isatty(fileno(this->debug_attr.ostream->file)) ) { \
    fprintf(this->debug_attr.ostream->file, "\033[1;33m[%-20s] " fmt "\033[0m", this->attributes.name,  ##__VA_ARGS__ ); \
  } else { \
    fprintf(this->debug_attr.ostream->file, "[%-20s] " fmt , this->attributes.name,  ##__VA_ARGS__ ); \
  } \
  } \
}

#define WARNING_MSG_APPEND( this, fmt, ... ) \
{ \
  if( isatty(fileno(this->debug_attr.ostream->file)) ) { \
    fprintf(this->debug_attr.ostream->file, "\033[1;33m" fmt "\033[0m", ##__VA_ARGS__ ); \
  } else { \
    fprintf(this->debug_attr.ostream->file, fmt , ##__VA_ARGS__ ); \
  } \
}

#define DEBUG_EXEC( this, _level, ... ) \
  if( this->debug_attr.debug && (this->debug_attr.level >= _level) ) __VA_ARGS__; 
#endif /* __cplusplus */


#endif /* __DEBUG_COMMON_H__ */
