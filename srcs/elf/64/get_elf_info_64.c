/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_elf_info_64.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/14 00:14:31 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/25 22:28:18 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"
#include "libft.h"

extern uint32_t	woody64_size;

void			get_elf_info_64(t_env *e, t_elf64 *elf)
{
	char		*section_name;

	ft_memset(elf, 0, sizeof(*elf));
	elf->header = (void *)e->file;
	elf->program = (void *)(e->file + elf->header->e_phoff);
	elf->section = (void *)(e->file + elf->header->e_shoff);
	if (elf->header->e_shstrndx <= SHN_UNDEF || elf->header->e_shstrndx > elf->header->e_shnum)
		ft_fatal("String table out of file. Section \".text\" unreachable. Abort.", e);
	/* Find the .text section */
	elf->string_table = (char *)(e->file + elf->section[elf->header->e_shstrndx].sh_offset);
	elf->text_section = NULL;
	for (size_t i = 0; i < elf->header->e_shnum; i++)
	{
		section_name = elf->string_table + elf->section[i].sh_name;
		if (ft_strcmp(section_name, ".text") == 0)
		{
			elf->text_section = &elf->section[i];
			break ;
		}
	}
	if (elf->text_section == NULL)
		ft_fatal("Section \".text\" not found.", e);
	elf->text_size = elf->text_section->sh_size;
	/* Find the .text segment */
	elf->text_program = NULL;
	for (size_t i = 0; i < elf->header->e_phnum; i++)
	{
		if (elf->text_section->sh_addr >= elf->program[i].p_vaddr
			&& elf->text_section->sh_addr < elf->program[i].p_vaddr + elf->program[i].p_filesz)
		{
			elf->text_program = &elf->program[i];
			break;
		}
	}
	if (elf->text_program == NULL)
		ft_fatal("Segment \".text\" not found.", e);
	e->woody_total_size = woody64_size
		+ sizeof(e->key)                                              /* generated key for decryption */
		+ sizeof(elf->text_offset)                                    /* .text section offset         */
		+ sizeof(elf->text_size)                                      /* .text section size           */
		+ sizeof(elf->old_entry)                                      /* program entry point          */
		+ sizeof(uint64_t)                                            /* banner size                  */
		+ ((e->banner && *e->banner) ? ft_strlen(e->banner) + 1 : 0); /* banner                       */
	e->modulo = 16 - (e->woody_total_size % 16);
	e->woody_total_size += e->modulo;
}
