/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pack_elf_64.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/05/11 15:41:56 by root              #+#    #+#             */
/*   Updated: 2020/12/24 09:07:11 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

static void		find_text_first_section(t_elf64 *elf, Elf64_Shdr **first)
{
	*first = NULL;
	for (size_t i = 0; i < elf->header->e_shnum; i++)
	{
		if (elf->section[i].sh_addr >= elf->text_program->p_vaddr
			&& elf->section[i].sh_addr < elf->text_program->p_vaddr + elf->text_program->p_filesz)
		{
			/* Find first section of the .text segment */
			if (*first == NULL || elf->section[i].sh_addr < (*first)->sh_addr)
				*first = elf->section + i;
		}
	}
}

static void		find_text_next_segment(t_elf64 *elf, Elf64_Phdr **next)
{
	*next = NULL;
	for (size_t i = 0; i < elf->header->e_phnum; i++)
	{
		if (elf->program[i].p_vaddr >= elf->text_program->p_vaddr + elf->text_program->p_filesz)
		{
			if (*next == NULL || elf->program[i].p_vaddr < (*next)->p_vaddr)
				*next = elf->program + i;
		}
	}
}

void			pack_elf_64(t_env *e)
{
	t_elf64		elf;
	Elf64_Phdr	*next;
	Elf64_Shdr	*first;
	size_t		room_before_next;
	size_t		room_before_first;

	get_elf_info_64(e, &elf);
	find_text_first_section(&elf, &first);
	find_text_next_segment(&elf, &next);
	room_before_next = (next) ? next->p_vaddr - (elf.text_program->p_vaddr + elf.text_program->p_filesz) : 0;
	room_before_first = first->sh_addr - elf.text_program->p_vaddr;
	/* Check if we have room to write our code between the '.text' segment and the next segment */
	if (room_before_next > e->woody_total_size)
		modification_after_text_64(e, &elf);
	/* Or before the first .text segment section */
	else if (room_before_first > e->woody_total_size)
		modification_before_text_64(e, &elf);
	/* Otherwise, enhance .text segment size */
	else
		modification_add_padding_64(e, &elf);
	write_new_file_64(e, &elf);
}
