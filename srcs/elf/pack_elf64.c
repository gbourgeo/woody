/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pack_elf64.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/05/11 15:41:56 by root              #+#    #+#             */
/*   Updated: 2020/11/22 13:14:01 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include "libft.h"
#include "main.h"

extern uint32_t 	woody64_size;
void				woody64_func(void);
void				woody64_encrypt(u_char *data, size_t len, const uint32_t *key);

static void			encrypt_text_section(t_env *e, t_elf64 *elf);
static void			write_new_file(t_env *e, t_elf64 *elf);
static void			write_after_text(t_env *e, t_elf64 *elf);
static void			write_before_text(t_env *e, t_elf64 *elf);
static void			write_add_padding(t_env *e, t_elf64 *elf);

void				pack_elf64(t_env *e)
{
	t_elf64		elf;
	
	ft_memset(&elf, 0, sizeof(elf));
	elf.header = (Elf64_Ehdr *)e->file;
	elf.section = (Elf64_Shdr *)(e->file + elf.header->e_shoff);
	elf.program = (Elf64_Phdr *)(e->file + elf.header->e_phoff);
	e->woody_datalen = ((e->banner && *e->banner) ? ft_strlen(e->banner) + 1 : 0)
		+ sizeof(size_t)				/* banner length */
		+ sizeof(elf.old_entry)			/* program entry point */
		+ sizeof(elf.text_crypted_size)	/* .text section size */
		+ sizeof(elf.text_entry)		/* .text section offset */
		+ sizeof(e->key);				/* generated key for decryption */
	encrypt_text_section(e, &elf);
	write_new_file(e, &elf);
}

static void			encrypt_text_section(t_env *e, t_elf64 *elf)
{
	char		*string_table;
	char		*section_name;
	Elf64_Addr	vaddr;

	if (elf->header->e_shstrndx == SHN_UNDEF)
		ft_fatal("String table not set. Section \".text\" unreachable.", e);

	/* Find the .text section */
	string_table = (char *)(e->file + elf->section[elf->header->e_shstrndx].sh_offset);
	elf->text_section = NULL;
	for (size_t i = 0; i < elf->header->e_shnum; i++) {
		section_name = string_table + elf->section[i].sh_name;
		if (ft_strcmp(section_name, ".text") == 0) {
			elf->text_section = &elf->section[i];
			break ;
		}
	}
	if (elf->text_section == NULL)
		ft_fatal("Section \".text\" not found.", e);
	elf->text_crypted_size = elf->text_section->sh_size;

	/* Find the .text segment (for later) */
	elf->text_program = NULL;
	vaddr = 0;
	for (size_t i = 0, vsize = 0; i < elf->header->e_phnum; i++) {
		if (elf->program[i].p_type == PT_LOAD) {
			vaddr = elf->program[i].p_vaddr;
			vsize = elf->program[i].p_vaddr + elf->program[i].p_filesz;
			if (elf->text_section->sh_addr >= vaddr &&
				elf->text_section->sh_addr < vsize) {
				elf->text_program = &elf->program[i];
				break ;
			}
		}
	}
	if (elf->text_program == NULL)
		ft_fatal("Program header containing section \".text\" not found.", e);

	/* Encrypt the .text section */
	u_char *text = (u_char *)(e->file + elf->text_section->sh_offset);
	woody64_encrypt(text, elf->text_section->sh_size, e->key);
}

static Elf64_Phdr	*find_next_segment(Elf64_Phdr *prog, Elf64_Phdr *programs, size_t phnum)
{
	Elf64_Phdr		*next;

	next = NULL;
	for (size_t i = 0; i < phnum; i++) {
		if (programs[i].p_vaddr > prog->p_vaddr)
			if (next == NULL || programs[i].p_vaddr < next->p_vaddr)
				next = programs + i;
	}
	return (next);
}

static Elf64_Shdr	*find_first_section(Elf64_Phdr *prog, Elf64_Shdr *sections, size_t shnum)
{
	Elf64_Shdr		*sec;

	sec = NULL;
	for (size_t i = 0; i < shnum; i++) {
		if (sections[i].sh_addr >= prog->p_vaddr)
			if (sec == NULL || sections[i].sh_addr < sec->sh_addr)
				sec = sections + i;
	}
	return (sec);
}

static void			write_new_file(t_env *e, t_elf64 *elf)
{
	Elf64_Phdr *next;
	Elf64_Shdr *sec;

	next = find_next_segment(elf->text_program, elf->program, elf->header->e_phnum);
	sec = find_first_section(elf->text_program, elf->section, elf->header->e_shnum);
	e->fd = open(OUTPUT_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 00755);
	if (e->fd == -1)
		ft_fatal(NULL, e);
	/* Check if we have space to write our code between the '.text' PT_LOAD segment and the next PT_LOAD segment */
	if (next && next->p_offset - (elf->text_program->p_offset + elf->text_program->p_filesz) > woody64_size + e->woody_datalen)
		write_after_text(e, elf);
	/* Or before the first .text program section */
	else if (sec && sec->sh_addr - elf->text_program->p_vaddr > woody64_size + e->woody_datalen)
		write_before_text(e, elf);
	/* Otherwise, enhance segment size */
	else
		write_add_padding(e, elf);
	close(e->fd);
	e->fd = 0;
}

static void			write_after_text(t_env *e, t_elf64 *elf)
{
	char		*ptr;
	size_t		banner_size;

# ifdef DEBUG
	printf("\e[32mWrite AFTER text ...\e[0m\n");
# endif
	ptr = (char *)e->file;
	banner_size = (e->banner && *e->banner) ? ft_strlen(e->banner) + 1 : 0;

	e->off = elf->text_program->p_offset + elf->text_program->p_filesz;
	elf->old_entry = (elf->text_program->p_vaddr + elf->text_program->p_memsz - elf->header->e_entry) * (-1);
	elf->text_entry = (elf->text_program->p_vaddr + elf->text_program->p_memsz - elf->text_section->sh_addr) * (-1);
	elf->header->e_entry = elf->text_program->p_vaddr + elf->text_program->p_memsz;

	elf->text_program->p_memsz += (woody64_size + e->woody_datalen);
	elf->text_program->p_filesz += (woody64_size + e->woody_datalen);
	elf->text_program->p_flags = PF_R | PF_W | PF_X;

/* Had this line if you want to disassemble the infection with debuggers */
	// elf->text_section->sh_size += woody64_size;
	write(e->fd, ptr, e->off);
	write(e->fd, &woody64_func, woody64_size);
	write(e->fd, e->key, sizeof(e->key));
	write(e->fd, &elf->text_entry, sizeof(elf->text_entry));
	write(e->fd, &elf->text_crypted_size, sizeof(elf->text_crypted_size));
	write(e->fd, &elf->old_entry, sizeof(elf->old_entry));
	write(e->fd, &banner_size, sizeof(banner_size));
	if (e->banner && *e->banner)
	{
		write(e->fd, e->banner, banner_size - 1);
		write(e->fd, "\n", 1);
	}
	e->off += (woody64_size + e->woody_datalen);
	write(e->fd, ptr + e->off, e->file_size - e->off - 1);
}

static void		write_before_text(t_env *e, t_elf64 *elf)
{
	char		*ptr;
	size_t		banner_size;
	int			mod;

# ifdef DEBUG
	printf("\e[34mWrite BEFORE text ...\e[0m\n");
# endif
	ptr = (char *)e->file;
	banner_size = (e->banner && *e->banner) ? ft_strlen(e->banner) + 1 : 0;
	mod = (woody64_size + e->woody_datalen) % 16;
	/* 
	** If we get here, it means we already ADD padding and write infection before the first section of the .text segment.
	** All we have to do is chack if we have space between the entry point and the start of the segment.
	** If not we ADD padding again.
	*/
	if (elf->header->e_entry - elf->text_program->p_vaddr < woody64_size + e->woody_datalen + mod)
		return (write_add_padding(e, elf));
	/*
	** Otherwise, we can write our code before
	*/

	/* Offset from where we will write our code */
	e->off = elf->header->e_entry - (woody64_size + e->woody_datalen + mod);
	/* Old entry point */
	elf->old_entry = woody64_size + e->woody_datalen + mod;
	/* '.text' section offset from woody */
	elf->text_entry = elf->text_section->sh_addr - e->off;
	/* New entry point */
	elf->header->e_entry = e->off;

	elf->text_program->p_flags = PF_R | PF_W | PF_X;

	write(e->fd, ptr, e->off);
	write(e->fd, &woody64_func, woody64_size);
	write(e->fd, e->key, sizeof(e->key));
	write(e->fd, &elf->text_entry, sizeof(elf->text_entry));
	write(e->fd, &elf->text_crypted_size, sizeof(elf->text_crypted_size));
	write(e->fd, &elf->old_entry, sizeof(elf->old_entry));
	write(e->fd, &banner_size, sizeof(banner_size));
	if (e->banner && *e->banner)
	{
		write(e->fd, e->banner, banner_size - 1);
		write(e->fd, "\n", 1);
	}
	e->off += (woody64_size + e->woody_datalen);
	write(e->fd, ptr + e->off, e->file_size - e->off - 1);
}

static void		write_add_padding(t_env *e, t_elf64 *elf)
{
	char		*ptr;
	size_t		banner_size;
	size_t		padding;
	int			mod;

# ifdef DEBUG
	printf("\e[33mWrite ADD padding ...\e[0m\n");
# endif
	ptr = (char *)e->file;
	banner_size = (e->banner && *e->banner) ? ft_strlen(e->banner) + 1 : 0;
	mod = (woody64_size + e->woody_datalen) % 16;
	/* Align woody entry point to 16 */
	padding = 0;
	while (padding < woody64_size + e->woody_datalen + mod)
		padding += elf->text_program->p_align;
	/* Change Program Header offset */
	for (size_t i = 0; i < elf->header->e_phnum; i++) {
		if (elf->program[i].p_offset >= elf->text_program->p_offset + elf->text_program->p_filesz) {
			if (elf->text_program->p_vaddr + elf->text_program->p_memsz >= elf->program[i].p_vaddr)
				ft_fatal("new Segment size too large. Risk of rewriting other Segment(s). Abort.", e);
			elf->program[i].p_offset += padding;
			elf->program[i].p_vaddr += padding;
			elf->program[i].p_paddr += padding;
		}
	}
	/* Change symbol, dynamic and relocation offsets */
	for (size_t i = 0; i < elf->header->e_shnum; i++) {
		/* SYMBOL */
		if (elf->section[i].sh_type == SHT_SYMTAB || elf->section[i].sh_type == SHT_DYNSYM) {
			size_t		symbol_addr;
			Elf64_Sym	*symbol;

			symbol_addr = (size_t)e->file + elf->section[i].sh_offset;
			symbol = (Elf64_Sym *)symbol_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++) {
				if (symbol->st_value >= elf->text_program->p_offset) {
					symbol->st_value += padding;
				}
				symbol_addr += elf->section[i].sh_entsize;
				symbol = (Elf64_Sym *)symbol_addr;
			}
		}
		/* DYNAMIC */
		if (elf->section[i].sh_type == SHT_DYNAMIC) {
			size_t		dynamic_addr;
			Elf64_Dyn	*dynamic;

			dynamic_addr = (size_t)e->file + elf->section[i].sh_offset;
			dynamic = (Elf64_Dyn *)dynamic_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++) {
				if (dynamic->d_un.d_val >= elf->text_program->p_offset
				&& dynamic->d_un.d_val < 0x8000000) { // <- usefull ???
					dynamic->d_un.d_val += padding;
				}
				dynamic_addr += elf->section[i].sh_entsize;
				dynamic = (Elf64_Dyn *)dynamic_addr;
			}
		}
		/* RELOCATION */
		if (elf->section[i].sh_type == SHT_REL) {
			size_t		rel_addr;
			Elf64_Rel	*rel;

			rel_addr = (size_t)e->file + elf->section[i].sh_offset;
			rel = (Elf64_Rel *)rel_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++) {
				if (rel->r_offset >= elf->text_program->p_offset) {
					rel->r_offset += padding;
				}
				rel_addr += elf->section[i].sh_entsize;
				rel = (Elf64_Rel *)rel_addr;
			}
		}
		if (elf->section[i].sh_type == SHT_RELA) {
			size_t		rela_addr;
			Elf64_Rela	*rela;

			rela_addr = (size_t)e->file + elf->section[i].sh_offset;
			rela = (Elf64_Rela *)rela_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++) {
				if (rela->r_offset >= elf->text_program->p_offset) {
					rela->r_offset += padding;
				}
				if (rela->r_addend >= (long int)(elf->text_program->p_offset)) {
					rela->r_addend += padding;
				}
				rela_addr += elf->section[i].sh_entsize;
				rela = (Elf64_Rela *)rela_addr;
			}
		}
	}
	/* Change the .got.plt offsets */
	if (elf->header->e_shstrndx == SHN_UNDEF)
			ft_fatal("String table not reachable. Abort.", e);
	char	*string_table = (char *)e->file + (elf->section + elf->header->e_shstrndx)->sh_offset;

	for (size_t i = 0; i < elf->header->e_shnum; i++) {
		Elf64_Shdr	*shdr = (Elf64_Shdr *)(e->file + elf->header->e_shoff) + i;
		char		*name = string_table + shdr->sh_name;

		if (ft_strcmp(name, ".got.plt") == 0) {
			void	*ptr = (u_char *)e->file + shdr->sh_offset;
			for (size_t j = 0; j < shdr->sh_size; j += sizeof(unsigned long int)) {
				if (*(unsigned long int *)(ptr + j) >= elf->text_program->p_offset)
					*(unsigned long int *)(ptr + j) += padding;
			}
			break ;
		}
	}
	/* Change Section Header offest */
	for (size_t i = 0; i < elf->header->e_shnum; i++) {
		if (elf->section[i].sh_offset >= elf->text_program->p_offset) {
			elf->section[i].sh_offset += padding;
		}
		if (elf->section[i].sh_addr >= elf->text_program->p_offset) {
			elf->section[i].sh_addr += padding;
		}
	}
	elf->header->e_shoff += padding;
	elf->text_program->p_memsz += padding;
	elf->text_program->p_filesz += padding;
	elf->text_program->p_flags = PF_R | PF_W | PF_X;

	/* Woody entry point */
	Elf64_Addr	woody_off = elf->text_program->p_vaddr + padding - woody64_size - e->woody_datalen - mod;
	/* Offset of '.text' PT_LOAD */
	e->off = elf->text_program->p_offset;
	/* Old entry point offset from woody */
	elf->old_entry = elf->header->e_entry + padding - woody_off;
	/* '.text' section offset from woody */
	elf->text_entry = elf->text_section->sh_addr - woody_off;
	/* New program entry point */
	elf->header->e_entry = woody_off;

	write(e->fd, ptr, e->off);
	while (padding-- > woody64_size + e->woody_datalen + mod)
		write(e->fd, "\0", 1);
	write(e->fd, &woody64_func, woody64_size);
	write(e->fd, e->key, sizeof(e->key));
	write(e->fd, &elf->text_entry, sizeof(elf->text_entry));
	write(e->fd, &elf->text_crypted_size, sizeof(elf->text_crypted_size));
	write(e->fd, &elf->old_entry, sizeof(elf->old_entry));
	write(e->fd, &banner_size, sizeof(banner_size));
	if (e->banner && *e->banner)
	{
		write(e->fd, e->banner, banner_size - 1);
		write(e->fd, "\n", 1);
	}
	while (mod--)
		write(e->fd, "\0", 1);
	write(e->fd, ptr + e->off, e->file_size - e->off - 1);
}
