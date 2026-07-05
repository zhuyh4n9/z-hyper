#ifndef __ZHYPER_ERRNO_H__
#define __ZHYPER_ERRNO_H__

// POSIX error codes

#define EPERM           1  /* Operation not permitted */
#define ENOENT          2  /* No such file or directory */
#define ESRCH           3  /* No such process */
#define EINTR           4  /* Interrupted system call */
#define EIO             5  /* I/O error */
#define ENXIO           6  /* No such device or address */
#define E2BIG           7  /* Argument list too long */
#define ENOEXEC         8  /* Exec format error */

#define EBADF           10 /* Bad file descriptor */

#define EAGAIN          11 /* Try again */
#define ENOMEM          12 /* Out of memory */
#define EACCES          13 /* Permission denied */

#define EBUSY           16 /* Device or resource busy */
#define EINVAL          22 /* Invalid argument */

#define ERANGE          34 /* Result too large */

#define EPROTO         71 /* Protocol error */

#define EOVERFLOW       75 /* Value too large for defined data type */

#define EFAULT         77 /* Bad address */

#define ENOTSUP        95 /* Operation not supported */

#define ESTALE          116 /* Structure needs cleaning */


#endif  // __ZHYPER_ERRNO_H__