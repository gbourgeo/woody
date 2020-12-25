/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/06/11 04:59:30 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/25 19:55:45 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WOODY_H
# define WOODY_H

# include <fcntl.h>
# include <unistd.h>
# include <sys/mman.h>
# include <sys/types.h>
# ifdef __linux__
#  include <elf.h>
#  ifndef u_char
#   define u_char unsigned char
#  endif
# elif __APPLE__
#  include <mach-o/loader.h>
# endif

# define OUTPUT_FILENAME	"woody"

typedef struct	s_env
{
	const char	*progname;
	char		banner[256];
	uint64_t	banner_len;
	ssize_t		file_size;
	void		*file;
	uint32_t	key[4];
	size_t		woody_total_size;
	size_t		off;
	size_t		padding;
	int			fd;
	int			modulo;
}				t_env;

# ifdef __linux__
typedef struct	s_elf32
{
	Elf32_Ehdr	*header;
	Elf32_Phdr	*program;
	Elf32_Shdr	*section;
	char		*string_table;
	Elf32_Shdr	*text_section;
	Elf32_Phdr	*text_program;
	Elf32_Addr	old_entry;
	Elf32_Addr	text_offset;
	uint32_t	text_size;
	u_char		padding[8];
}				t_elf32;

typedef struct	s_elf64
{
	Elf64_Ehdr	*header;
	Elf64_Phdr	*program;
	Elf64_Shdr	*section;
	char		*string_table;
	Elf64_Shdr	*text_section;
	Elf64_Phdr	*text_program;
	Elf64_Addr	old_entry;
	Elf64_Addr	text_offset;
	uint64_t	text_size;
}				t_elf64;

void			pack_elf_32(t_env *e);
void			get_elf_info_32(t_env *e, t_elf32 *elf);
void			modification_after_text_32(t_env *e, t_elf32 *elf);
void			modification_before_text_32(t_env *e, t_elf32 *elf);
void			modification_add_padding_32(t_env *e, t_elf32 *elf);
void			write_new_file_32(t_env *e, t_elf32 *elf);

void			pack_elf_64(t_env *e);
void			get_elf_info_64(t_env *e, t_elf64 *elf);
void			modification_after_text_64(t_env *e, t_elf64 *elf);
void			modification_before_text_64(t_env *e, t_elf64 *elf);
void			modification_add_padding_64(t_env *e, t_elf64 *elf);
void			write_new_file_64(t_env *e, t_elf64 *elf);

# ifndef u_char
typedef unsigned char		u_char;
# endif

# elif __APPLE__
typedef struct					s_macho64
{
	struct mach_header_64		*header;
	struct segment_command_64	*segment;
	struct section_64			*section;
	struct entry_point_command	*entry;
	struct segment_command_64	*segtext;
	struct section_64			*sectext;
	struct segment_command_64	*lastseg;
	uint64_t					old_entryoff;
	uint64_t					new_entryoff;
	uint64_t					sectext_size;
	uint64_t					text_entryoff;
	uint64_t					segtext_size;
	uint64_t					lastsect_off;
	uint64_t					filesz;
	uint64_t					sectsize;
}								t_macho64;

void			check_macho_info(t_env *e);
void			pack_macho64(t_env *e);
uint32_t		byteswap_32(uint32_t x);

# endif

int				ft_fatal(char *str, t_env *e);
void			generate_new_key(uint32_t key[4]);
void			check_elf_info(t_env *e);

#endif
