// ============================================================================
// fs.c - user FileSytem API
// ============================================================================

#include "fs.h"
#include "bfs.h"

// ============================================================================
// Close the file currently open on file descriptor 'fd'.
// ============================================================================
i32 fsClose(i32 fd) {
  i32 inum = bfsFdToInum(fd);
  bfsDerefOFT(inum);
  return 0;
}

// ============================================================================
// Create the file called 'fname'.  Overwrite, if it already exsists.
// On success, return its file descriptor.  On failure, EFNF
// ============================================================================
i32 fsCreate(str fname) {
  i32 inum = bfsCreateFile(fname);
  if (inum == EFNF)
    return EFNF;
  return bfsInumToFd(inum);
}

// ============================================================================
// Format the BFS disk by initializing the SuperBlock, Inodes, Directory and
// Freelist.  On succes, return 0.  On failure, abort
// ============================================================================
i32 fsFormat() {
  FILE *fp = fopen(BFSDISK, "w+b");
  if (fp == NULL)
    FATAL(EDISKCREATE);

  i32 ret = bfsInitSuper(fp); // initialize Super block
  if (ret != 0) {
    fclose(fp);
    FATAL(ret);
  }

  ret = bfsInitInodes(fp); // initialize Inodes block
  if (ret != 0) {
    fclose(fp);
    FATAL(ret);
  }

  ret = bfsInitDir(fp); // initialize Dir block
  if (ret != 0) {
    fclose(fp);
    FATAL(ret);
  }

  ret = bfsInitFreeList(); // initialize Freelist
  if (ret != 0) {
    fclose(fp);
    FATAL(ret);
  }

  fclose(fp);
  return 0;
}

// ============================================================================
// Mount the BFS disk.  It must already exist
// ============================================================================
i32 fsMount() {
  FILE *fp = fopen(BFSDISK, "rb");
  if (fp == NULL)
    FATAL(ENODISK); // BFSDISK not found
  fclose(fp);
  return 0;
}

// ============================================================================
// Open the existing file called 'fname'.  On success, return its file
// descriptor.  On failure, return EFNF
// ============================================================================
i32 fsOpen(str fname) {
  i32 inum = bfsLookupFile(fname); // lookup 'fname' in Directory
  if (inum == EFNF)
    return EFNF;
  return bfsInumToFd(inum);
}

// ============================================================================
// Read 'numb' bytes of data from the cursor in the file currently fsOpen'd on
// File Descriptor 'fd' into 'buf'.  On success, return actual number of bytes
// read (may be less than 'numb' if we hit EOF).  On failure, abort
// ============================================================================
i32 fsRead(i32 fd, i32 numb, void *buf) {

  // ++++++++++++++++++++++++
  // Insert your code here
  // ++++++++++++++++++++++++
  // get the inum and cursor
  i32 inum = bfsFdToInum(fd);
  i32 ofte = bfsFindOFTE(inum);
  i32 curs = g_oft[ofte].curs;
  i32 size = bfsGetSize(inum);
  i32 num_blocks = (size + BYTESPERBLOCK - 1) / BYTESPERBLOCK;
  if (curs >= size)
    return 0;
  if (curs + numb > size)
    numb = size - curs;

  i32 fbn = curs / BYTESPERBLOCK;

  i32 offset = curs % BYTESPERBLOCK;

  i32 bytes_read = 0;
  if (offset != 0) {
    i8 temp[BYTESPERBLOCK];
    i32 ret = bfsRead(inum, fbn, temp);
    if (ret != 0)
      return 0;
    if (numb < BYTESPERBLOCK - offset) {
      memcpy(buf, temp + offset, numb);
      g_oft[ofte].curs += numb;
      return numb;
    }
    memcpy(buf, temp + offset, BYTESPERBLOCK - offset);
  }

  while (bytes_read < numb) {
    if (fbn >= num_blocks)
      break;
    if (numb - bytes_read < BYTESPERBLOCK)
      break;
    i8 temp[BYTESPERBLOCK];
    i32 ret = bfsRead(inum, fbn, temp);
    if (ret != 0)
      return 0;
    memcpy(buf + bytes_read, temp, BYTESPERBLOCK);
    g_oft[ofte].curs += BYTESPERBLOCK;
    bytes_read += BYTESPERBLOCK;
    fbn++;
  }

  if ((numb - bytes_read < BYTESPERBLOCK) && bytes_read < numb) {
    i8 temp[BYTESPERBLOCK];
    i32 ret = bfsRead(inum, fbn, temp);
    if (ret != 0)
      return 0;
    memcpy(buf + bytes_read, temp, numb - bytes_read);
    g_oft[ofte].curs += numb - bytes_read;
    bytes_read += numb - bytes_read;
  }

  return bytes_read;
}

// ============================================================================
// Move the cursor for the file currently open on File Descriptor 'fd' to the
// byte-offset 'offset'.  'whence' can be any of:
//
//  SEEK_SET : set cursor to 'offset'
//  SEEK_CUR : add 'offset' to the current cursor
//  SEEK_END : add 'offset' to the size of the file
//
// On success, return 0.  On failure, abort
// ============================================================================
i32 fsSeek(i32 fd, i32 offset, i32 whence) {

  if (offset < 0)
    FATAL(EBADCURS);

  i32 inum = bfsFdToInum(fd);
  i32 ofte = bfsFindOFTE(inum);

  switch (whence) {
  case SEEK_SET:
    g_oft[ofte].curs = offset;
    break;
  case SEEK_CUR:
    g_oft[ofte].curs += offset;
    break;
  case SEEK_END: {
    i32 end = fsSize(fd);
    g_oft[ofte].curs = end + offset;
    break;
  }
  default:
    FATAL(EBADWHENCE);
  }
  return 0;
}

// ============================================================================
// Return the cursor position for the file open on File Descriptor 'fd'
// ============================================================================
i32 fsTell(i32 fd) { return bfsTell(fd); }

// ============================================================================
// Retrieve the current file size in bytes.  This depends on the highest offset
// written to the file, or the highest offset set with the fsSeek function.  On
// success, return the file size.  On failure, abort
// ============================================================================
i32 fsSize(i32 fd) {
  i32 inum = bfsFdToInum(fd);
  return bfsGetSize(inum);
}

// ============================================================================
// Write 'numb' bytes of data from 'buf' into the file currently fsOpen'd on
// filedescriptor 'fd'.  The write starts at the current file offset for the
// destination file.  On success, return 0.  On failure, abort
// ============================================================================
i32 fsWrite(i32 fd, i32 numb, void *buf) {

  // ++++++++++++++++++++++++
  // Insert your code here
  // +++++++++++++++++++++++
  i32 inum = bfsFdToInum(fd);
  i32 ofte = bfsFindOFTE(inum);
  i32 cursor = g_oft[ofte].curs;
  i32 size = bfsGetSize(inum);

  i32 fbn = cursor / BYTESPERBLOCK;
  i32 last = (cursor + numb) / BYTESPERBLOCK;
  if (cursor + numb > size) {
    bfsExtend(inum, (cursor + numb) / BYTESPERBLOCK);
    bfsSetSize(inum, cursor + numb);
  }
  i8 temp[BYTESPERBLOCK];
  i32 bytes_written = 0;
  i32 curr = cursor;

  while (fbn <= last) {
    i32 dbn = bfsFbnToDbn(inum, fbn);
    i32 ret = bfsRead(inum, fbn, temp);
    if (ret != 0)
      return 0;
    i32 start = curr - (fbn * BYTESPERBLOCK);
    i32 end;
    if (curr + (numb - (curr - cursor)) > ((fbn + 1) * BYTESPERBLOCK))
      end = BYTESPERBLOCK;
    else
      end = (numb - (curr - cursor)) + start;
    memcpy(temp + start, buf + (curr - cursor), end - start);
    ret = bioWrite(dbn, temp);
    g_oft[ofte].curs += end - start;
    bytes_written += end - start;
    fbn++;
    curr += end - start;
  }
  return 0;
}
