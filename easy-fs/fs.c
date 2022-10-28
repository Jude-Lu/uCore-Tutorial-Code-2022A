/**
 * @file fs.c
 * @brief File system implementation.  
 * @details
 * Five layers: \n
 * + Blocks: allocator for raw disk blocks. \n
 * + Log: crash recovery for multi-step updates. \n
 * + Files: inode allocator, reading, writing, metadata. \n
 * + Directories: inode with special contents (list of other inodes!) \n
 * + Names: paths like /usr/rtm/xv6/fs.c for convenient naming. \n
 * 
 * This file contains the low-level file system manipulation \n
 * routines.  The (higher-level) system call implementations \n
 * are in sysfile.c.
 */

#include "fs.h"
#include "file.h"
#include "log.h"

extern struct FSManager *fs_manager;

/// there should be one superblock per disk device, but we run with
/// only one device
struct superblock sb;

/// Read the super block.
static void readsb(int dev, struct superblock *sb)
{
	void *bp;
	bp = (fs_manager->bread)(dev, 1);
	memmove(sb, fs_manager->buf_data(bp), sizeof(*sb));
	(fs_manager->brelse)(bp);
}

/// Init fs
void fsinit()
{
	int dev = ROOTDEV;
	readsb(dev, &sb);
	if (sb.magic != FSMAGIC) {
		panic("invalid file system");
	}
}

/// Zero a block.
static void bzero(int dev, int bno)
{
	void *bp;
	bp = (fs_manager->bread)(dev, bno);
	memset(fs_manager->buf_data(bp), 0, BSIZE);
	(fs_manager->bwrite)(bp);
	(fs_manager->brelse)(bp);
}

// Blocks.

/// Allocate a zeroed disk block.
static uint balloc(uint dev)
{
	int b, bi, m;
	void *bp;

	bp = 0;
	for (b = 0; b < sb.size; b += BPB) {
		bp = (fs_manager->bread)(dev, BBLOCK(b, sb));
		for (bi = 0; bi < BPB && b + bi < sb.size; bi++) {
			m = 1 << (bi % 8);
			if (((fs_manager->buf_data(bp))[bi / 8] & m) == 0) { // Is block free?
				(fs_manager->buf_data(bp))[bi / 8] |= m; // Mark block in use.
				(fs_manager->bwrite)(bp);
				(fs_manager->brelse)(bp);
				bzero(dev, b + bi);
				return b + bi;
			}
		}
		(fs_manager->brelse)(bp);
	}
	panic("balloc: out of blocks");
	return 0;
}

/// Free a disk block.
static void bfree(int dev, uint b)
{
	void *bp;
	int bi, m;

	bp = (fs_manager->bread)(dev, BBLOCK(b, sb));
	bi = b % BPB;
	m = 1 << (bi % 8);
	if (((fs_manager->buf_data(bp))[bi / 8] & m) == 0)
		panic("freeing free block");
	(fs_manager->buf_data(bp))[bi / 8] &= ~m;
	(fs_manager->bwrite)(bp);
	(fs_manager->brelse)(bp);
}

///The inode table in memory
struct {
	struct inode inode[NINODE];
} itable;

static struct inode *iget(uint dev, uint inum);

/// Allocate an inode on device dev.
/// Mark it as allocated by  giving it type `type`.
/// Returns an allocated and referenced inode.
struct inode *ialloc(uint dev, short type)
{
	int inum;
	void *bp;
	struct dinode *dip;

	for (inum = 1; inum < sb.ninodes; inum++) {
		bp = (fs_manager->bread)(dev, IBLOCK(inum, sb));
		dip = (struct dinode *)(fs_manager->buf_data(bp)) + inum % IPB;
		if (dip->type == 0) { // a free inode
			memset(dip, 0, sizeof(*dip));
			dip->type = type;
			(fs_manager->bwrite)(bp);
			(fs_manager->brelse)(bp);
			return iget(dev, inum);
		}
		(fs_manager->brelse)(bp);
	}
	panic("ialloc: no inodes");
	return 0;
}

/// Copy a modified in-memory inode to disk.
/// Must be called after every change to an ip->xxx field
/// that lives on disk.
void iupdate(struct inode *ip)
{
	void *bp;
	struct dinode *dip;

	bp = (fs_manager->bread)(ip->dev, IBLOCK(ip->inum, sb));
	dip = (struct dinode *)(fs_manager->buf_data(bp)) + ip->inum % IPB;
	dip->type = ip->type;
	dip->size = ip->size;
	// LAB4: you may need to update link count here
	memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
	(fs_manager->bwrite)(bp);
	(fs_manager->brelse)(bp);
}

/// Find the inode with number inum on device dev
/// and return the in-memory copy. Does not read
/// it from disk.
static struct inode *iget(uint dev, uint inum)
{
	struct inode *ip, *empty;
	// Is the inode already in the table?
	empty = 0;
	for (ip = &itable.inode[0]; ip < &itable.inode[NINODE]; ip++) {
		if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
			ip->ref++;
			return ip;
		}
		if (empty == 0 && ip->ref == 0) ///< Remember empty slot.
			empty = ip;
	}

	// Recycle an inode entry.
	if (empty == 0)
		panic("iget: no inodes");

	ip = empty;
	ip->dev = dev;
	ip->inum = inum;
	ip->ref = 1;
	ip->valid = 0;
	return ip;
}

/// Increment reference count for ip.
/// Returns ip to enable ip = idup(ip1) idiom.
struct inode *idup(struct inode *ip)
{
	ip->ref++;
	return ip;
}

/// Reads the inode from disk if necessary.
void ivalid(struct inode *ip)
{
	void *bp;
	struct dinode *dip;
	if (ip->valid == 0) {
		bp = (fs_manager->bread)(ip->dev, IBLOCK(ip->inum, sb));
		dip = (struct dinode *)(fs_manager->buf_data(bp)) + ip->inum % IPB;
		ip->type = dip->type;
		ip->size = dip->size;
		// LAB4: You may need to get lint count here
		memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
		(fs_manager->brelse)(bp);
		ip->valid = 1;
		if (ip->type == 0)
			panic("ivalid: no type");
	}
}

/**
 * Drop a reference to an in-memory inode. \n
 * If that was the last reference, the inode table entry can
 * be recycled.
 * If that was the last reference and the inode has no links
 * to it, free the inode (and its content) on disk. \n
 * All calls to iput() must be inside a transaction in
 * case it has to free the inode.
 */
void iput(struct inode *ip)
{
	// LAB4: Unmark the condition and change link count variable name (nlink) if needed
	if (ip->ref == 1 && ip->valid && 0 /*&& ip->nlink == 0*/) {
		// inode has no links and no other references: truncate and free.
		itrunc(ip);
		ip->type = 0;
		iupdate(ip);
		ip->valid = 0;
	}
	ip->ref--;
}

/// Return the disk block address of the nth block in inode ip.
/// If there is no such block, bmap allocates one.
static uint bmap(struct inode *ip, uint bn)
{
	uint addr, *a;
	void *bp;

	if (bn < NDIRECT) {
		if ((addr = ip->addrs[bn]) == 0)
			ip->addrs[bn] = addr = balloc(ip->dev);
		return addr;
	}
	bn -= NDIRECT;

	if (bn < NINDIRECT) {
		// Load indirect block, allocating if necessary.
		if ((addr = ip->addrs[NDIRECT]) == 0)
			ip->addrs[NDIRECT] = addr = balloc(ip->dev);
		bp = (fs_manager->bread)(ip->dev, addr);
		a = (uint *)(fs_manager->buf_data(bp));
		if ((addr = a[bn]) == 0) {
			a[bn] = addr = balloc(ip->dev);
			(fs_manager->bwrite)(bp);
		}
		(fs_manager->brelse)(bp);
		return addr;
	}

	panic("bmap: out of range");
	return 0;
}

/// Truncate inode (discard contents).
void itrunc(struct inode *ip)
{
	int i, j;
	void *bp;
	uint *a;

	for (i = 0; i < NDIRECT; i++) {
		if (ip->addrs[i]) {
			bfree(ip->dev, ip->addrs[i]);
			ip->addrs[i] = 0;
		}
	}

	if (ip->addrs[NDIRECT]) {
		bp = (fs_manager->bread)(ip->dev, ip->addrs[NDIRECT]);
		a = (uint *)(fs_manager->buf_data(bp));
		for (j = 0; j < NINDIRECT; j++) {
			if (a[j])
				bfree(ip->dev, a[j]);
		}
		(fs_manager->brelse)(bp);
		bfree(ip->dev, ip->addrs[NDIRECT]);
		ip->addrs[NDIRECT] = 0;
	}

	ip->size = 0;
	iupdate(ip);
}

/// Read data from inode.
/// If user_dst==1, then dst is a user virtual address;
/// otherwise, dst is a kernel address.
int readi(struct inode *ip, int user_dst, uint64 dst, uint off, uint n)
{
	uint tot, m;
	void *bp;

	if (off > ip->size || off + n < off)
		return 0;
	if (off + n > ip->size)
		n = ip->size - off;

	for (tot = 0; tot < n; tot += m, off += m, dst += m) {
		bp = (fs_manager->bread)(ip->dev, bmap(ip, off / BSIZE));
		m = MIN(n - tot, BSIZE - off % BSIZE);
		if ((fs_manager->either_copyout)((fs_manager->get_curr_pagetable)(), user_dst, dst,
				   (char *)(fs_manager->buf_data(bp)) + (off % BSIZE), m) == -1) {
			(fs_manager->brelse)(bp);
			tot = -1;
			break;
		}
		(fs_manager->brelse)(bp);
	}
	return tot;
}

/**
 * Write data to inode. \n
 * Caller must hold ip->lock. \n
 * If user_src==1, then src is a user virtual address;
 * otherwise, src is a kernel address. \n
 * Returns the number of bytes successfully written. \n
 * If the return value is less than the requested n,
 * there was an error of some kind.
 */
int writei(struct inode *ip, int user_src, uint64 src, uint off, uint n)
{
	uint tot, m;
	void *bp;

	if (off > ip->size || off + n < off)
		return -1;
	if (off + n > MAXFILE * BSIZE)
		return -1;

	for (tot = 0; tot < n; tot += m, off += m, src += m) {
		bp = (fs_manager->bread)(ip->dev, bmap(ip, off / BSIZE));
		m = MIN(n - tot, BSIZE - off % BSIZE);
		if ((fs_manager->either_copyin)((fs_manager->get_curr_pagetable)(), user_src, src,
				  (char *)(fs_manager->buf_data(bp)) + (off % BSIZE), m) == -1) {
			(fs_manager->brelse)(bp);
			break;
		}
		(fs_manager->bwrite)(bp);
		(fs_manager->brelse)(bp);
	}

	if (off > ip->size)
		ip->size = off;

	// write the i-node back to disk even if the size didn't change
	// because the loop above might have called bmap() and added a new
	// block to ip->addrs[].
	iupdate(ip);

	return tot;
}

/// Look for a directory entry in a directory.
/// If found, set *poff to byte offset of entry.
struct inode *dirlookup(struct inode *dp, char *name, uint *poff)
{
	uint off, inum;
	struct dirent de;

	if (dp->type != T_DIR)
		panic("dirlookup not DIR");

	for (off = 0; off < dp->size; off += sizeof(de)) {
		if (readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
			panic("dirlookup read");
		if (de.inum == 0)
			continue;
		if (strncmp(name, de.name, DIRSIZ) == 0) {
			// entry matches path element
			if (poff)
				*poff = off;
			inum = de.inum;
			return iget(dp->dev, inum);
		}
	}

	return 0;
}

/// Show the filenames of all files in the directory
int dirls(struct inode *dp)
{
	uint64 off, count;
	struct dirent de;

	if (dp->type != T_DIR)
		panic("dirlookup not DIR");

	count = 0;
	for (off = 0; off < dp->size; off += sizeof(de)) {
		if (readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
			panic("dirlookup read");
		if (de.inum == 0)
			continue;
		printf("%s\n", de.name);
		count++;
	}
	return count;
}

/// Write a new directory entry (name, inum) into the directory dp.
int dirlink(struct inode *dp, char *name, uint inum)
{
	int off;
	struct dirent de;
	struct inode *ip;
	// Check that name is not present.
	if ((ip = dirlookup(dp, name, 0)) != 0) {
		iput(ip);
		return -1;
	}

	// Look for an empty dirent.
	for (off = 0; off < dp->size; off += sizeof(de)) {
		if (readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
			panic("dirlink read");
		if (de.inum == 0)
			break;
	}
	strncpy(de.name, name, DIRSIZ);
	de.inum = inum;
	if (writei(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
		panic("dirlink");
	return 0;
}

// LAB4: You may want to add dirunlink here

/// Return the inode of the root directory
struct inode *root_dir()
{
	struct inode *r = iget(ROOTDEV, ROOTINO);
	ivalid(r);
	return r;
}

/// Find the corresponding inode according to the path
struct inode *namei(char *path)
{
	int skip = 0;
	// if(path[0] == '.' && path[1] == '/')
	//     skip = 2;
	// if (path[0] == '/') {
	//     skip = 1;
	// }
	struct inode *dp = root_dir();
	if (dp == 0)
		panic("fs dumped.\n");
	return dirlookup(dp, path + skip, 0);
}
