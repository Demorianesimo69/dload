/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999, 2001, 2003  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * The patch about "ext3 with 256-byte inode size" was made by
 * Stefan Lippers-Hollmann <s.L-H@gmx.de> for Debian in 2008-01-30.
 * Thank Barton for submittal of this patch.
 * 						---- Tinybit, 2008-06-24
 */

/*
 * The ext4 patch comes from Gentoo.
 * Thank kraml for direction to this patch.
 * 						---- Tinybit, 2009-02-11
 */

#ifdef FSYS_EXT2FS

#include "shared.h"
#include "filesys.h"
#include "iamath.h"
#include "term.h"

static int mapblock1, mapblock2;

/* sizes are always in bytes, BLOCK values are always in DEV_BSIZE (sectors) */
#define DEV_BSIZE 512

/* include/linux/fs.h */
#define BLOCK_SIZE 1024		/* initial block size for superblock read */
/* made up, defaults to 1 but can be passed via mount_opts */
#define WHICH_SUPER 1
/* kind of from fs/ext2/super.c */
#define SBLOCK (WHICH_SUPER * BLOCK_SIZE / DEV_BSIZE)	/* = 2 */

/* include/asm-i386/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;
typedef __signed__ short __s16;
typedef unsigned short __u16;
typedef __signed__ int __s32;
typedef unsigned int __u32;
typedef unsigned long long __u64;

/*
 * Constants relative to the data blocks, from ext2_fs.h
 */
#define EXT2_NDIR_BLOCKS                12
#define EXT2_IND_BLOCK                  EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK                 (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK                 (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS                   (EXT2_TIND_BLOCK + 1)

/* include/linux/ext2_fs.h */
struct ext2_super_block
  {
    __u32 s_inodes_count;	/* Inodes count */
    __u32 s_blocks_count_lo;	/* Blocks count */
    __u32 s_r_blocks_count_lo;	/* Reserved blocks count */
    __u32 s_free_blocks_count_lo;	/* Free blocks count */
    __u32 s_free_inodes_count;	/* Free inodes count */
    __u32 s_first_data_block;	/* First Data Block */
    __u32 s_log_block_size;	/* Block size */
    __s32 s_obso_log_frag_size;	/* Obsoleted Fragment size */
    __u32 s_blocks_per_group;	/* # Blocks per group */
    __u32 s_obso_frags_per_group;	/* Obsoleted Fragments per group */
    __u32 s_inodes_per_group;	/* # Inodes per group */
    __u32 s_mtime;		/* Mount time */
    __u32 s_wtime;		/* Write time */
    __u16 s_mnt_count;		/* Mount count */
    __s16 s_max_mnt_count;	/* Maximal mount count */
    __u16 s_magic;		/* Magic signature */
    __u16 s_state;		/* File system state */
    __u16 s_errors;		/* Behaviour when detecting errors */
    __u16 s_minor_rev_level;	/* minor revision level */
    __u32 s_lastcheck;		/* time of last check */
    __u32 s_checkinterval;	/* max. time between checks */
    __u32 s_creator_os;		/* OS */
    __u32 s_rev_level;		/* Revision level */
    __u16 s_def_resuid;		/* Default uid for reserved blocks */
    __u16 s_def_resgid;		/* Default gid for reserved blocks */
    //__u32 s_reserved[235];	/* Padding to the end of the block */
    /*
     * These fields are for EXT2_DYNAMIC_REV superblocks only.
     *
     * Note: the difference between the compatible feature set and
     * the incompatible feature set is that if there is a bit set
     * in the incompatible feature set that the kernel doesn't
     * know about, it should refuse to mount the filesystem.
     *
     * e2fsck's requirements are more strict; if it doesn't know
     * about a feature in either the compatible or incompatible
     * feature set, it must abort and not try to meddle with
     * things it doesn't understand...
     */
    __u32 s_first_ino;		/* First non-reserved inode */
    __u16 s_inode_size;		/* size of inode structure */
    __u16 s_block_group_nr;	/* block group # of this superblock */
    __u32 s_feature_compat;	/* compatible feature set */
    __u32 s_feature_incompat;	/* incompatible feature set */
    __u32 s_feature_ro_compat;	/* readonly-compatible feature set */
    __u8 s_uuid[16];		/* 128-bit uuid for volume */
    char s_volume_name[16];	/* volume name */
    char s_last_mounted[64];	/* directory where last mounted */
    __u32 s_algorithm_usage_bitmap;	/* For compression */
    /*
     * Performance hints. Directory preallocation should only
     * happen if the EXT2_FEATURE_COMPAT_DIR_PREALLOC flag is on.
     */
    __u8 s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
    __u8 s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
    __u16 s_reserved_gdt_blocks;/* Per group table for online growth */
    /*
     * Journaling support valid if EXT2_FEATURE_COMPAT_HAS_JOURNAL set.
     */
    __u8 s_journal_uuid[16];	/* uuid of journal superblock */
    __u32 s_journal_inum;	/* inode number of journal file */
    __u32 s_journal_dev;	/* device number of journal file */
    __u32 s_last_orphan;	/* start of list of inodes to delete */
    __u32 s_hash_seed[4];	/* HTREE hash seed */
    __u8 s_def_hash_version;	/* Default hash version to use */
    __u8 s_jnl_backup_type;	/* Default type of journal backup */
    __u16 s_desc_size;		/* size of group descriptor */
    __u32 s_default_mount_opts;
    __u32 s_first_meta_bg;	/* First metablock group */
    __u32 s_mkfs_time;		/* When the filesystem was created */
    __u32 s_jnl_blocks[17];	/* Backup of the journal inode */
//    __u32 s_reserved[172];	/* Padding to the end of the block */
		/* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
		__u32	s_blocks_count_hi;	/* Blocks count */
		__u32	s_r_blocks_count_hi;	/* Reserved blocks count */
		__u32	s_free_blocks_count_hi;	/* Free blocks count */
		__u16	s_min_extra_isize;	/* All inodes have at least # bytes */
		__u16	s_want_extra_isize; 	/* New inodes should reserve # bytes */
		__u32	s_flags;		/* Miscellaneous flags */
		__u16 s_raid_stride;		/* RAID stride */
		__u16 s_mmp_interval;   /* # seconds to wait in MMP checking */
		__u64 s_mmp_block;      /* Block for multi-mount protection */
		__u32 s_raid_stripe_width; /* blocks on all data disks (N*stride)*/
		__u8 s_log_groups_per_flex;/* FLEX_BG group size */
		__u8 s_reserved_char_pad2;
		__u16 s_reserved_pad;
		__u64 s_kbytes_written;	/* nr of lifetime kilobytes written */
		__u32 s_reserved[160];        /* Padding to the end of the block */
  } __attribute__ ((packed));

struct ext4_group_desc
  {
    __u32 bg_block_bitmap_lo;	/* Blocks bitmap block */
    __u32 bg_inode_bitmap_lo;	/* Inodes bitmap block */
    __u32 bg_inode_table_lo;	/* Inodes table block */
    __u16 bg_free_blocks_count_lo;	/* Free blocks count */
    __u16 bg_free_inodes_count_lo;	/* Free inodes count */
    __u16 bg_used_dirs_count_lo;	/* Directories count */
    __u16 bg_flags;		/* EXT4_BG_flags (INODE_UNINIT, etc) */
    __u32 bg_reserved[2];		/* Likely block/inode bitmap checksum */
    __u16 bg_itable_unused_lo;	/* Unused inodes count */
    __u16 bg_checksum;		/* crc16(sb_uuid+group+desc) */
    __u32 bg_block_bitmap_hi;	/* Blocks bitmap block MSB */
    __u32 bg_inode_bitmap_hi;	/* Inodes bitmap block MSB */
    __u32 bg_inode_table_hi;	/* Inodes table block MSB */
    __u16 bg_free_blocks_count_hi;/* Free blocks count MSB */
    __u16 bg_free_inodes_count_hi;/* Free inodes count MSB */
    __u16 bg_used_dirs_count_hi;	/* Directories count MSB */
    __u16 bg_itable_unused_hi;	/* Unused inodes count MSB */
    __u32 bg_reserved2[3];
  } __attribute__ ((packed));

struct ext2_inode
  {
    __u16 i_mode;		/* File mode */
    __u16 i_uid_lo;		/* Owner Uid */
    __u32 i_size_lo;		/* 4: Size in bytes */
    __u32 i_atime;		/* Access time */
    __u32 i_ctime;		/* 12: Creation time */
    __u32 i_mtime;		/* Modification time */
    __u32 i_dtime;		/* 20: Deletion Time */
    __u16 i_gid_lo;		/* Group Id */
    __u16 i_links_count;	/* 24: Links count */
    __u32 i_blocks_lo;		/* Blocks count */
    __u32 i_flags;		/* 32: File flags */
    union
      {
	struct
	  {
	    __u32 l_i_reserved1;
	  }
	linux1;
	struct
	  {
	    __u32 h_i_translator;
	  }
	hurd1;
	struct
	  {
	    __u32 m_i_reserved1;
	  }
	masix1;
      }
    osd1;			/* OS dependent 1 */
    __u32 i_block[EXT2_N_BLOCKS];	/* 40: Pointers to blocks */
    __u32 i_version_lo;		/* File version (for NFS) */
    __u32 i_file_acl_lo;		/* File ACL */
    __u32 i_size_high;
    __u32 i_obso_faddr;		/* Obsoleted fragment address */
    union
      {
	struct
	  {
		__u16	l_i_blocks_high; /* were l_i_reserved1 */
		__u16	l_i_file_acl_high;
		__u16	l_i_uid_high;	/* these 2 fields */
		__u16	l_i_gid_high;	/* were reserved2[0] */
		__u32	l_i_reserved2;
	  }
	linux2;
	struct
	  {
		__u16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
	    __u16 h_i_mode_high;
	    __u16 h_i_uid_high;
	    __u16 h_i_gid_high;
	    __u32 h_i_author;
	  }
	hurd2;
	struct
	  {
		__u16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
		__u16	m_i_file_acl_high;
	    __u32 m_i_reserved2[2];
	  }
	masix2;
      }
    osd2;			/* OS dependent 2 */
	__u16	i_extra_isize;
	__u16	i_pad1;
	__u32  i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
	__u32  i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
	__u32  i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
	__u32  i_crtime;       /* File Creation time */
	__u32  i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
	__u32  i_version_hi;	/* high 32 bits for 64-bit version */
  } __attribute__ ((packed));

#define EXT4_FEATURE_INCOMPAT_META			0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS		0x0040 /* extents support */
#define EXT4_FEATURE_INCOMPAT_64BIT			0x0080 /* grub not supported*/
#define EXT4_FEATURE_INCOMPAT_MMP           0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG		0x0200

#define EXT4_HAS_INCOMPAT_FEATURE(sb,mask)			\
	( sb->s_feature_incompat & mask )

#define EXT4_EXTENTS_FL		0x00080000 /* Inode uses extents */
#define EXT4_HUGE_FILE_FL	0x00040000 /* Set to each huge file */

#define EXT4_MIN_DESC_SIZE			32

/* linux/limits.h */
#define NAME_MAX         255	/* # chars in a file name */

/* linux/posix_type.h */
typedef int linux_off_t;

/* linux/ext2fs.h */
#define EXT2_NAME_LEN 255
struct ext2_dir_entry
  {
    __u32 inode;		/* Inode number */
    __u16 rec_len;		/* Directory entry length */
    __u8 name_len;		/* Name length */
    __u8 file_type;
    char name[EXT2_NAME_LEN];	/* File name */
  } __attribute__ ((packed));

/* linux/ext4_fs_extents.h */ 
/* This is the extent on-disk structure.
 * It's used at the bottom of the tree.
 */
struct ext4_extent 
  {
	__u32  ee_block;   /* first logical block extent covers */
	__u16  ee_len;     /* number of blocks covered by extent */
	__u16  ee_start_hi;    /* high 16 bits of physical block */
	__u32  ee_start_lo;    /* low 32 bits of physical block */
  } __attribute__ ((packed));

/*
 * This is index on-disk structure.
 * It's used at all the levels except the bottom.
 */
struct ext4_extent_idx 
  {
    __u32  ei_block;   /* index covers logical blocks from 'block' */
    __u32  ei_leaf_lo; /* pointer to the physical block of the next *
	                     * level. leaf or next index could be there */
    __u16  ei_leaf_hi; /* high 16 bits of physical block */
    __u16  ei_unused;
  } __attribute__ ((packed));

/*
 * Each block (leaves and indexes), even inode-stored has header.
 */
struct ext4_extent_header 
  {
    __u16  eh_magic;   /* probably will support different formats */
    __u16  eh_entries; /* number of valid entries */
    __u16  eh_max;     /* capacity of store in entries */
    __u16  eh_depth;   /* has tree real underlying blocks? */
    __u32  eh_generation;  /* generation of the tree */
  } __attribute__ ((packed));

#define EXT4_EXT_MAGIC      (0xf30a)
#define EXT_FIRST_EXTENT(__hdr__) \
    ((struct ext4_extent *) (((char *) (__hdr__)) +     \
                 sizeof(struct ext4_extent_header)))
#define EXT_FIRST_INDEX(__hdr__) \
    ((struct ext4_extent_idx *) (((char *) (__hdr__)) + \
                 sizeof(struct ext4_extent_header)))
#define EXT_LAST_EXTENT(__hdr__) \
    (EXT_FIRST_EXTENT((__hdr__)) + (__u16)((__hdr__)->eh_entries) - 1)
#define EXT_LAST_INDEX(__hdr__) \
    (EXT_FIRST_INDEX((__hdr__)) + (__u16)((__hdr__)->eh_entries) - 1)



/* linux/ext2fs.h */
/*
 * EXT2_DIR_PAD defines the directory entries boundaries
 *
 * NOTE: It must be a multiple of 4
 */
#define EXT2_DIR_PAD                    4
#define EXT2_DIR_ROUND                  (EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)      (((name_len) + 8 + EXT2_DIR_ROUND) & \
                                         ~EXT2_DIR_ROUND)


/* ext2/super.c */
//#define log2(n) ffz(~(n))

#define EXT2_SUPER_MAGIC      0xEF53	/* include/linux/ext2_fs.h */
#define EXT2_ROOT_INO              2	/* include/linux/ext2_fs.h */
#define PATH_MAX                1024	/* include/linux/limits.h */
#define MAX_LINK_COUNT             5	/* number of symbolic links to follow */


/* made up, these are pointers into FSYS_BUF */
/* read once, always stays there: */
#define NAME_BUF ((char *)(FSYS_BUF))	/* 512 bytes */
#define SUPERBLOCK \
    ((struct ext2_super_block *)((FSYS_BUF)+512))	/* 1024 bytes */
#define GROUP_DESC \
    ((struct ext2_group_desc *) \
     ((grub_size_t)SUPERBLOCK + sizeof(struct ext2_super_block)))	/* 32 bytes */
#define INODE \
    ((struct ext2_inode *)((grub_size_t)GROUP_DESC + EXT2_BLOCK_SIZE(SUPERBLOCK)))
#define DATABLOCK1 \
    ((grub_size_t)((grub_size_t)INODE + sizeof(struct ext2_inode)))
#define DATABLOCK2 \
    ((grub_size_t)((grub_size_t)DATABLOCK1 + EXT2_BLOCK_SIZE(SUPERBLOCK)))

/* linux/ext2_fs.h */
#define EXT2_ADDR_PER_BLOCK(s)          (EXT2_BLOCK_SIZE(s) / sizeof (__u32))
#define EXT2_ADDR_PER_BLOCK_BITS(s)	(log2_tmp(EXT2_ADDR_PER_BLOCK(s)))

/* Revision levels */
#define EXT2_GOOD_OLD_REV	0	/* The good old (original) format */
#define EXT2_DYNAMIC_REV	1	/* V2 format w/ dynamic inode sizes */

#define EXT2_CURRENT_REV	EXT2_GOOD_OLD_REV
#define EXT2_MAX_SUPP_REV	EXT2_DYNAMIC_REV

#define EXT2_GOOD_OLD_INODE_SIZE	128

#define EXT2_INODE_SIZE(s)	(((s)->s_rev_level == EXT2_GOOD_OLD_REV) ? EXT2_GOOD_OLD_INODE_SIZE : (s)->s_inode_size)
//#define EXT2_INODE_SIZE(s)		(SUPERBLOCK->s_inode_size)
#define EXT2_INODES_PER_BLOCK(s)	(EXT2_BLOCK_SIZE(s)/EXT2_INODE_SIZE(s))

/* linux/ext2_fs.h */
#define EXT2_BLOCK_SIZE_BITS(s)        ((s)->s_log_block_size + 10)
/* kind of from ext2/super.c */
#define EXT2_BLOCK_SIZE(s)	(1 << EXT2_BLOCK_SIZE_BITS(s))
/* linux/ext2fs.h */
/* sizeof(struct ext2_group_desc) is changed in ext4 
 * in kernel code, ext2/3 uses sizeof(struct ext2_group_desc) to calculate 
 * number of desc per block, while ext4 uses superblock->s_desc_size in stead
 * superblock->s_desc_size is not available in ext2/3
 * */
#define EXT2_DESC_SIZE(s) \
	(EXT4_HAS_INCOMPAT_FEATURE(s,EXT4_FEATURE_INCOMPAT_64BIT)? \
	s->s_desc_size : EXT4_MIN_DESC_SIZE)
#define EXT2_DESC_PER_BLOCK(s) \
	(EXT2_BLOCK_SIZE(s) / EXT2_DESC_SIZE(s))
#define EXT4_META_GROUP_SIZE(s) \
	(EXT4_HAS_INCOMPAT_FEATURE(s,EXT4_FEATURE_INCOMPAT_META)? \
	(EXT2_BLOCK_SIZE(s) / EXT2_DESC_SIZE(s)) : 0)

/* linux/stat.h */
#define S_IFMT  00170000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFDIR  0040000
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)

#define linkbuf ((char *)(FSYS_BUF - PATH_MAX))	/* buffer for following symbolic links */
/* include/asm-i386/bitops.h */
/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */

/* check filesystem types and read superblock into memory buffer */
int ext2fs_mount (void);
int
ext2fs_mount (void)
{
  if ((unsigned int)part_length < (SBLOCK + (sizeof(struct ext2_super_block) / DEV_BSIZE)))
      return 0;

  if (!devread(SBLOCK, 0, sizeof(struct ext2_super_block), (unsigned long long)(grub_size_t) (char *)SUPERBLOCK, 0xedde0d90))
      return 0;

  if (SUPERBLOCK->s_magic != EXT2_SUPER_MAGIC)
      return 0;

  if (SUPERBLOCK->s_inodes_count == 0)
      return 0;

	if ((SUPERBLOCK->s_blocks_count_lo | SUPERBLOCK->s_blocks_count_hi) == 0)
      return 0;

  if (SUPERBLOCK->s_blocks_per_group == 0)
      return 0;

  if (SUPERBLOCK->s_inodes_per_group == 0)
      return 0;

  if (SUPERBLOCK->s_first_data_block > 1) /* 1 for 1K block, 0 otherwise */
      return 0;

  if (SUPERBLOCK->s_log_block_size > 4)	/* max size of block is 16K. 0 for 1K */
      return 0;

  if (SUPERBLOCK->s_first_data_block)
  {
	if (SUPERBLOCK->s_log_block_size)
		return 0;
  } else {
	if (SUPERBLOCK->s_log_block_size == 0)
		return 0;
  }

  if (SUPERBLOCK->s_rev_level)
  {
	if (SUPERBLOCK->s_inode_size == 0)
		return 0;
	if (EXT2_BLOCK_SIZE(SUPERBLOCK) % (SUPERBLOCK->s_inode_size))
		return 0;
  }

  return 1;
}

/* Takes a file system block number and reads it into BUFFER. */
static int ext2_rdfsb (unsigned long long fsblock, unsigned long long buffer);
static int
ext2_rdfsb (unsigned long long fsblock, unsigned long long buffer)
{
#ifdef E2DEBUG
  printf ("fsblock %d buffer %d\n", fsblock, buffer);
#endif /* E2DEBUG */
  return devread (fsblock * (EXT2_BLOCK_SIZE (SUPERBLOCK) / DEV_BSIZE), 0,
      EXT2_BLOCK_SIZE (SUPERBLOCK), buffer, 0xedde0d90);
}

/* from
  ext2/inode.c:ext2_bmap()
*/
/* Maps LOGICAL_BLOCK (the file offset divided by the blocksize) into
   a physical block (the location in the file system) via an inode. */
static int ext2fs_block_map (unsigned int logical_block);
static int
ext2fs_block_map (unsigned int logical_block)
{

#ifdef E2DEBUG
  unsigned char *i;
  for (i = (unsigned char *) INODE;
       i < ((unsigned char *) INODE + sizeof (struct ext2_inode));
       i++)
    {
      printf ("%c", "0123456789abcdef"[*i >> 4]);
      printf ("%c", "0123456789abcdef"[*i % 16]);
      if (!((i + 1 - (unsigned char *) INODE) % 16))
	{
	  printf ("\n");
	}
      else
	{
	  printf (" ");
	}
    }
  printf ("logical block %d\n", logical_block);
#endif /* E2DEBUG */

  /* if it is directly pointed to by the inode, return that physical addr */
  if (logical_block < EXT2_NDIR_BLOCKS)
    {
#ifdef E2DEBUG
      printf ("returning %d\n", (unsigned char *) (INODE->i_block[logical_block]));
      printf ("returning %d\n", INODE->i_block[logical_block]);
#endif /* E2DEBUG */
      return INODE->i_block[logical_block];
    }
  /* else */
  logical_block -= EXT2_NDIR_BLOCKS;
  /* try the indirect block */
  if (logical_block < EXT2_ADDR_PER_BLOCK (SUPERBLOCK))
    {
      if (mapblock1 != 1
	  && !ext2_rdfsb (INODE->i_block[EXT2_IND_BLOCK], DATABLOCK1))
	{
	  errnum = ERR_FSYS_CORRUPT;
	  return -1;
	}
      mapblock1 = 1;
      return ((__u32 *)(grub_size_t) DATABLOCK1)[logical_block];
    }
  /* else */
  logical_block -= EXT2_ADDR_PER_BLOCK (SUPERBLOCK);
  /* now try the double indirect block */
  if (logical_block < (unsigned int)(1 << (EXT2_ADDR_PER_BLOCK_BITS (SUPERBLOCK) * 2)))
    {
      int bnum;
      if (mapblock1 != 2
	  && !ext2_rdfsb (INODE->i_block[EXT2_DIND_BLOCK], DATABLOCK1))
	{
	  errnum = ERR_FSYS_CORRUPT;
	  return -1;
	}
      mapblock1 = 2;
      if ((bnum = (((__u32 *)(grub_size_t) DATABLOCK1)
		   [logical_block >> EXT2_ADDR_PER_BLOCK_BITS (SUPERBLOCK)]))
	  != mapblock2
	  && !ext2_rdfsb (bnum, DATABLOCK2))
	{
	  errnum = ERR_FSYS_CORRUPT;
	  return -1;
	}
      mapblock2 = bnum;
      return ((__u32 *)(grub_size_t) DATABLOCK2)
	[logical_block & (EXT2_ADDR_PER_BLOCK (SUPERBLOCK) - 1)];
    }
  /* else */
  mapblock2 = -1;
  logical_block -= (1 << (EXT2_ADDR_PER_BLOCK_BITS (SUPERBLOCK) * 2));
  
  if (mapblock1 != 3
      && !ext2_rdfsb (INODE->i_block[EXT2_TIND_BLOCK], DATABLOCK1))
    {
      errnum = ERR_FSYS_CORRUPT;
      return -1;
    }
  
  mapblock1 = 3;
  
  if (!ext2_rdfsb (((__u32 *)(grub_size_t) DATABLOCK1)
		   [logical_block >> (EXT2_ADDR_PER_BLOCK_BITS (SUPERBLOCK)
				      * 2)],
		   DATABLOCK2))
    {
      errnum = ERR_FSYS_CORRUPT;
      return -1;
    }
  
  if (! ext2_rdfsb (((__u32 *)(grub_size_t) DATABLOCK2)
		   [(logical_block >> EXT2_ADDR_PER_BLOCK_BITS (SUPERBLOCK))
		    & (EXT2_ADDR_PER_BLOCK (SUPERBLOCK) - 1)],
		   DATABLOCK2))
    {
      errnum = ERR_FSYS_CORRUPT;
      return -1;
    }
  
  return ((__u32 *)(grub_size_t) DATABLOCK2)[logical_block & (EXT2_ADDR_PER_BLOCK (SUPERBLOCK) - 1)];
}

/* extent binary search index
 * find closest index in the current level extent tree
 * kind of from ext4_ext_binsearch_idx in ext4/extents.c
 */
static struct ext4_extent_idx* ext4_ext_binsearch_idx(struct ext4_extent_header* eh, int logical_block);
static struct ext4_extent_idx*
ext4_ext_binsearch_idx(struct ext4_extent_header* eh, int logical_block)
{
  struct ext4_extent_idx *r, *l, *m;
  l = EXT_FIRST_INDEX(eh) + 1;
  r = EXT_LAST_INDEX(eh);
  while (l <= r) 
    {
	  m = l + (r - l) / 2;
	  if (logical_block < (int)m->ei_block)
		  r = m - 1;
	  else
		  l = m + 1;
	}
  return (struct ext4_extent_idx*)(l - 1);
}

/* extent binary search 
 * find closest extent in the leaf level 
 * kind of from ext4_ext_binsearch in ext4/extents.c
 */
static struct ext4_extent* ext4_ext_binsearch(struct ext4_extent_header* eh, int logical_block);
static struct ext4_extent*
ext4_ext_binsearch(struct ext4_extent_header* eh, int logical_block)
{
  struct ext4_extent *r, *l, *m;
  l = EXT_FIRST_EXTENT(eh) + 1;
  r = EXT_LAST_EXTENT(eh);
  while (l <= r) 
    {
	  m = l + (r - l) / 2;
	  if (logical_block < (int)m->ee_block)
		  r = m - 1;
	  else
		  l = m + 1;
	}
  return (struct ext4_extent*)(l - 1);
}

/* Maps extents enabled logical block into physical block via an inode. 
 * EXT4_HUGE_FILE_FL should be checked before calling this.
 */
//static int
static unsigned long long ext4fs_block_map (int logical_block);
static unsigned long long
ext4fs_block_map (int logical_block)
{
  struct ext4_extent_header *eh;
  struct ext4_extent *ex;//, *extent;
  struct ext4_extent_idx *ei;//, *index;
  int depth;

#ifdef E2DEBUG
  unsigned char *i;
  for (i = (unsigned char *) INODE;
       i < ((unsigned char *) INODE + sizeof (struct ext2_inode));
       i++)
    {
      printf ("%c", "0123456789abcdef"[*i >> 4]);
      printf ("%c", "0123456789abcdef"[*i % 16]);
      if (!((i + 1 - (unsigned char *) INODE) % 16))
	{
	  printf ("\n");
	}
      else
	{
	  printf (" ");
	}
    }
  printf ("logical block %d\n", logical_block);
#endif /* E2DEBUG */
  eh = (struct ext4_extent_header*)INODE->i_block;
  if (eh->eh_magic != EXT4_EXT_MAGIC)
  {
          errnum = ERR_FSYS_CORRUPT;
          return -1;
  }
  while((depth = eh->eh_depth) != 0)
  	{ /* extent index */
	  if (eh->eh_magic != EXT4_EXT_MAGIC)
	  {
	          errnum = ERR_FSYS_CORRUPT;
		  return -1;
	  }
	  ei = ext4_ext_binsearch_idx(eh, logical_block);
		if (!ext2_rdfsb(((unsigned long long)ei->ei_leaf_hi<<32) + ei->ei_leaf_lo, DATABLOCK1))
	{
	  errnum = ERR_FSYS_CORRUPT;
	  return -1;
	}
	  eh = (struct ext4_extent_header*)(grub_size_t)DATABLOCK1;
  	}

  /* depth==0, we come to the leaf */
  ex = ext4_ext_binsearch(eh, logical_block);
//  if (ex->ee_start_hi) 
//	{/* 64bit physical block number not supported */
//	  errnum = ERR_FILELENGTH;
//	  return -1;
//	}
  if ((int)(ex->ee_block + ex->ee_len) < logical_block)
	{
	  errnum = ERR_FSYS_CORRUPT;
	  return -1;
	}
//  return ex->ee_start_lo + logical_block - ex->ee_block; 
	return ((unsigned long long)ex->ee_start_hi<<32) + ex->ee_start_lo + logical_block - ex->ee_block;
}

/* preconditions: all preconds of ext2fs_block_map */
unsigned long long ext2fs_read (unsigned long long buf, unsigned long long len, unsigned int write);
unsigned long long
ext2fs_read (unsigned long long buf, unsigned long long len, unsigned int write)
{
  unsigned int logical_block;
  unsigned int offset;
  unsigned int ret = 0;
  unsigned int size = 0;
	unsigned long long map;

#ifdef E2DEBUG
  static char hexdigit[] = "0123456789abcdef";
  unsigned char *i;
  
  for (i = (unsigned char *) INODE;
       i < ((unsigned char *) INODE + sizeof (struct ext2_inode));
       i++)
    {
      printf ("%c", hexdigit[*i >> 4]);
      printf ("%c", hexdigit[*i % 16]);
      
      if (! ((i + 1 - (unsigned char *) INODE) % 16))
	{
	  printf ("\n");
	}
      else
	{
	  printf (" ");
	}
    }
#endif /* E2DEBUG */
  while (len > 0)
  {
      /* find the (logical) block component of our location */
      logical_block = filepos >> EXT2_BLOCK_SIZE_BITS (SUPERBLOCK);
      offset = filepos & (EXT2_BLOCK_SIZE (SUPERBLOCK) - 1);

      /* map extents enabled logical block number to physical fs on-dick block number */
      if (EXT4_HAS_INCOMPAT_FEATURE(SUPERBLOCK,EXT4_FEATURE_INCOMPAT_EXTENTS) 
		&& INODE->i_flags & EXT4_EXTENTS_FL)
	map = ext4fs_block_map (logical_block);
      else
      map = ext2fs_block_map (logical_block);

#ifdef E2DEBUG
      printf ("map=%d\n", map);
#endif /* E2DEBUG */
      
      if ((long long)map < 0)
	  break;

      size = EXT2_BLOCK_SIZE (SUPERBLOCK);
      size -= offset;
      
      if (size > len)
	  size = len;

      if (map == 0)
      {
	  if (buf)
		grub_memset64 ((unsigned long long) buf, 0, size);
      } else {
          disk_read_func = disk_read_hook;

          devread (map * (EXT2_BLOCK_SIZE (SUPERBLOCK) / DEV_BSIZE),
		 offset, size, buf, write);

          disk_read_func = NULL;
      }

      if (buf)
	buf += size;
      len -= size;	/* len always >= 0 */
      filepos += size;
      ret += size;
  }

  if (errnum)
      ret = 0;

  return ret;
}

static inline int ext2_is_fast_symlink (void);
static inline
int ext2_is_fast_symlink (void)
{
  int ea_blocks;
  ea_blocks = INODE->i_file_acl_lo ? EXT2_BLOCK_SIZE (SUPERBLOCK) / DEV_BSIZE : 0;
  return (int)INODE->i_blocks_lo == ea_blocks;
}

/* preconditions: ext2fs_mount already executed, therefore supblk in buffer
 *   known as SUPERBLOCK
 * returns: 0 if error, nonzero iff we were able to find the file successfully
 * postconditions: on a nonzero return, buffer known as INODE contains the
 *   inode of the file we were trying to look up
 * side effects: messes up GROUP_DESC buffer area
 */
int ext2fs_dir (char *dirname);
int
ext2fs_dir (char *dirname)
{
  int current_ino = EXT2_ROOT_INO;	/* start at the root */
  int updir_ino = current_ino;	/* the parent of the current directory */
  int group_id,tem;			/* which group the inode is in */
	unsigned long long group_desc;		/* fs pointer to that group */
  int desc;			/* index within that group */
	unsigned long long ino_blk;			/* fs pointer of the inode's information */
  int str_chk = 0;		/* used to hold the results of a string compare */
  struct ext4_group_desc *ext4_gdp;
  struct ext2_inode *raw_inode;	/* inode info corresponding to current_ino */

//char linkbuf[PATH_MAX];	/* buffer for following symbolic links */
  int link_count = 0;

  char *rest;
  char ch;			/* temp char holder */

  int off;			/* offset within block of directory entry (off mod blocksize) */
	unsigned long long loc;			/* location within a directory */
  int blk;			/* which data blk within dir entry (off div blocksize) */
  int map;			/* fs pointer of a particular block from dir entry */
  struct ext2_dir_entry *dp;	/* pointer to directory entry */
#ifdef E2DEBUG
  unsigned char *i;
#endif	/* E2DEBUG */
	int empty = 0;

  /* loop invariants:
     current_ino = inode to lookup
     dirname = pointer to filename component we are cur looking up within
     the directory known pointed to by current_ino (if any)
   */

  while (1)
    {
#ifdef E2DEBUG
      printf ("inode %d\n", current_ino);
      printf ("dirname=%s\n", dirname);
#endif /* E2DEBUG */

      /* look up an inode */
      group_id = tem = (current_ino - 1) / SUPERBLOCK->s_inodes_per_group;
			if ((EXT4_HAS_INCOMPAT_FEATURE(SUPERBLOCK,EXT4_FEATURE_INCOMPAT_META))
				&& (group_id >= (int)SUPERBLOCK->s_first_meta_bg))
			{
				group_id = (group_id - SUPERBLOCK->s_first_meta_bg)
					% EXT4_META_GROUP_SIZE(SUPERBLOCK);
				group_desc = (group_id >> log2_tmp (EXT2_DESC_PER_BLOCK (SUPERBLOCK)))
					+ (tem - group_id) * SUPERBLOCK->s_blocks_per_group
					+ (SUPERBLOCK->s_first_meta_bg == 0 ?
					(WHICH_SUPER + SUPERBLOCK->s_first_data_block) : WHICH_SUPER);
			}
			else
			{
				group_desc = (group_id >> log2_tmp (EXT2_DESC_PER_BLOCK (SUPERBLOCK)))
					+ WHICH_SUPER + SUPERBLOCK->s_first_data_block;
			}
      desc = group_id & (EXT2_DESC_PER_BLOCK (SUPERBLOCK) - 1);
#ifdef E2DEBUG
      printf ("ipg=%d, dpb=%d\n", SUPERBLOCK->s_inodes_per_group,
	      EXT2_DESC_PER_BLOCK (SUPERBLOCK));
      printf ("group_id=%d group_desc=%d desc=%d\n", group_id, group_desc, desc);
#endif /* E2DEBUG */
			if (!ext2_rdfsb (group_desc,(grub_size_t) GROUP_DESC))	
	{
	  return 0;
	}
	  ext4_gdp = (struct ext4_group_desc *)( (__u8*)GROUP_DESC + 
			  		desc * EXT2_DESC_SIZE(SUPERBLOCK));

		ino_blk = (EXT4_HAS_INCOMPAT_FEATURE(SUPERBLOCK,EXT4_FEATURE_INCOMPAT_64BIT) ?
			((unsigned long long)ext4_gdp->bg_inode_table_hi<<32) : 0) + ext4_gdp->bg_inode_table_lo +
	(((current_ino - 1) % (SUPERBLOCK->s_inodes_per_group))
	 >> log2_tmp (EXT2_INODES_PER_BLOCK (SUPERBLOCK)));
#ifdef E2DEBUG
      printf ("inode table fsblock=%d\n", ino_blk);
#endif /* E2DEBUG */
      if (!ext2_rdfsb (ino_blk, (grub_size_t) INODE))
	{
	  return 0;
	}

      /* reset indirect blocks! */
      mapblock2 = mapblock1 = -1;

      raw_inode = (struct ext2_inode *)((char *)INODE +
	((current_ino - 1) & (EXT2_INODES_PER_BLOCK (SUPERBLOCK) - 1)) *
	 EXT2_INODE_SIZE (SUPERBLOCK));
#ifdef E2DEBUG
      printf ("ipb=%d, sizeof(inode)=%d\n",
	     EXT2_INODES_PER_BLOCK (SUPERBLOCK), EXT2_INODE_SIZE (SUPERBLOCK));
      printf ("inode=%x, raw_inode=%x\n", INODE, raw_inode);
      printf ("offset into inode table block=%d\n", ((grub_size_t) raw_inode - (grub_size_t) INODE));
      for (i = (unsigned char *) INODE; i <= (unsigned char *) raw_inode;
	   i++)
	{
	  printf ("%c", "0123456789abcdef"[*i >> 4]);
	  printf ("%c", "0123456789abcdef"[*i % 16]);
	  if (!((i + 1 - (unsigned char *) INODE) % 16))
	    {
	      printf ("\n");
	    }
	  else
	    {
	      printf (" ");
	    }
	}
      printf ("first word=%x\n", *((int *) raw_inode));
#endif /* E2DEBUG */

      /* copy inode to fixed location */
      memmove ((void *) INODE, (void *) raw_inode, sizeof (struct ext2_inode));

#ifdef E2DEBUG
      printf ("first word=%x\n", *((int *) INODE));
#endif /* E2DEBUG */

      /* If we've got a symbolic link, then chase it. */
      if (S_ISLNK (INODE->i_mode))
	{
	  int len;
	  if (++link_count > MAX_LINK_COUNT)
	    {
	      errnum = ERR_SYMLINK_LOOP;
	      return 0;
	    }

	  /* Find out how long our remaining name is. */
	  for (len = 0; (ch = dirname[len]) /*&& !isspace (ch)*/; len++)
	  {
	  }

	  /* Get the symlink size. */
		filemax = (INODE->i_size_lo);
	  if (filemax + len > PATH_MAX - 2)
	    {
	      errnum = ERR_FILELENGTH;
	      return 0;
	    }

	  if (len)
	    {
	      /* Copy the remaining name to the end of the symlink data.
	         Note that DIRNAME and LINKBUF may overlap! */
	      memmove (linkbuf + filemax, dirname, len);
	    }
	  linkbuf[filemax + len] = '\0';

	  /* Read the symlink data. 
	   * Slow symlink is extents enabled
	   * But since grub_read invokes ext2fs_read, nothing to change here
	   * */
	  if (! ext2_is_fast_symlink ())
	    {
	      /* Read the necessary blocks, and reset the file pointer. */
	      len = grub_read ((unsigned long long)(grub_size_t)linkbuf, filemax, 0xedde0d90);
	      filepos = 0;
	      if (!len)
		return 0;
	    }
	  else
	    {
	      /* Copy the data directly from the inode. 
		   * Fast symlink is not extents enabled
		   * */
	      len = filemax;
	      memmove (linkbuf, (char *) INODE->i_block, len);
	    }

#ifdef E2DEBUG
	  printf ("symlink=%s\n", linkbuf);
#endif

	  dirname = linkbuf;
	  if (*dirname == '/')
	    {
	      /* It's an absolute link, so look it up in root. */
	      current_ino = EXT2_ROOT_INO;
	      updir_ino = current_ino;
	    }
	  else
	    {
	      /* Relative, so look it up in our parent directory. */
	      current_ino = updir_ino;
	    }

	  /* Try again using the new name. */
	  continue;
	}

      /* if end of filename, INODE points to the file's inode */
      if (!*dirname || isspace (*dirname))
	{
	  if (!S_ISREG (INODE->i_mode))
	    {
	      errnum = ERR_BAD_FILETYPE;
	      return 0;
	    }
	  /* if file is too large, just stop and report an error*/
		filemax = ((unsigned long long)INODE->i_size_high<<32) + INODE->i_size_lo;
	  return 1;
	}

      /* else we have to traverse a directory */
      updir_ino = current_ino;

      /* skip over slashes */
      while (*dirname == '/')
	dirname++;

      /* if this isn't a directory of sufficient size to hold our file, abort */
	if (!(((unsigned long long)INODE->i_size_high<<32) + INODE->i_size_lo) || !S_ISDIR (INODE->i_mode))
	{
	  errnum = ERR_BAD_FILETYPE;
	  return 0;
	}

      /* skip to next slash or end of filename (space) */
      for (rest = dirname; (ch = *rest) /*&& !isspace (ch)*/ && ch != '/'; rest++)
      {

      }

      /* look through this directory and find the next filename component */
      /* invariant: rest points to slash after the next filename component */
      *rest = 0;
      loc = 0;

      do
	{

#ifdef E2DEBUG
	  printf ("dirname=%s, rest=%s, loc=%d\n", dirname, rest, loc);
#endif /* E2DEBUG */

	  /* if our location/byte offset into the directory exceeds the size,
	     give up */
	  if (loc >= (((unsigned long long)INODE->i_size_high<<32) + INODE->i_size_lo))
	    {
	      if (print_possibilities < 0)
		{
			if (!empty)
				return !(errnum = ERR_FILE_NOT_FOUND);
		  return 1;
		}
	      
	      errnum = ERR_FILE_NOT_FOUND;
	      *rest = ch;

	      return 0;
	    }

	  /* else, find the (logical) block component of our location */
	  /* ext4 logical block number the same as ext2/3 */
	  blk = loc >> EXT2_BLOCK_SIZE_BITS (SUPERBLOCK);

	  /* we know which logical block of the directory entry we are looking
	     for, now we have to translate that to the physical (fs) block on
	     the disk */
	  /* map extents enabled logical block number to physical fs on-dick block number */
	  if (EXT4_HAS_INCOMPAT_FEATURE(SUPERBLOCK,EXT4_FEATURE_INCOMPAT_EXTENTS) 
                        && INODE->i_flags & EXT4_EXTENTS_FL)
              map = ext4fs_block_map (blk);
	  else
	  map = ext2fs_block_map (blk);
#ifdef E2DEBUG
	  printf ("fs block=%d\n", map);
#endif /* E2DEBUG */
	  mapblock2 = -1;
	  if (map < 0) 
	  {
	      *rest = ch;
	      return 0;
	  }
          if (!ext2_rdfsb (map, DATABLOCK2))
	    {
	      errnum = ERR_FSYS_CORRUPT;
	      *rest = ch;
	      return 0;
	    }
	  off = loc & (EXT2_BLOCK_SIZE (SUPERBLOCK) - 1);
	  dp = (struct ext2_dir_entry *)(grub_size_t) (DATABLOCK2 + off);
	  /* advance loc prematurely to next on-disk directory entry  */
	  loc += dp->rec_len;

	  /* NOTE: ext2fs filenames are NOT null-terminated */

#ifdef E2DEBUG
	  printf ("directory entry ino=%d\n", dp->inode);
	  if (dp->inode)
	    printf ("entry=%s\n", dp->name);
#endif /* E2DEBUG */

	  if (dp->inode)
	    {
	      //int saved_c = dp->name[dp->name_len];
	      int j, k;
	      char ch1;
	      char *tmp_name= NAME_BUF;	/* EXT2_NAME_LEN is 255, so 512 byte buffer is needed. */

	      /* copy dp->name to tmp_name, and quote the spaces with a '\\' */
	      for (j = 0, k = 0; j < dp->name_len; j++)
	      {
		if (! (ch1 = dp->name[j]))
			break;

		tmp_name[k++] = ch1;
	      }
	      tmp_name[k] = 0;

	      str_chk = substring (dirname, tmp_name, 0);

	      if (print_possibilities && ch != '/'
		  && (!*dirname || str_chk <= 0))
		{
		  if (print_possibilities > 0)
		    print_possibilities = -print_possibilities;
			unsigned long long clo64 = current_color_64bit;
			unsigned int clo = current_color;	
			if (dp->file_type == 2)
			{
				if (current_term->setcolorstate)
					current_term->setcolorstate (COLOR_STATE_HIGHLIGHT);
				current_color_64bit = (current_color_64bit & 0xffffff) | (clo64 & 0xffffff00000000);
				current_color = (current_color & 0x0f) | (clo & 0xf0);
				console_setcolorstate (current_color | 0x100);	//设置控制台文本模式的颜色(UEFI)
			}
		  print_a_completion (tmp_name, 0);
			if (cursor_state & 1)
				current_term->setcolorstate (COLOR_STATE_STANDARD);
			else
				current_term->setcolorstate (COLOR_STATE_NORMAL);
			if (*tmp_name != 0x2e)
				empty = 1;
		}
	    }

	}
      while (!dp->inode || (str_chk || (print_possibilities && ch != '/')));

      current_ino = dp->inode;
      *(dirname = rest) = ch;
    }
  /* never get here */
}

#endif /* FSYS_EXT2_FS */
