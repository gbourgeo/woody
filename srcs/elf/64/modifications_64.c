/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   modifications_64.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/14 00:16:36 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/19 13:04:33 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"
#include "libft.h"

void			modification_after_text_64(t_env *e, t_elf64 *elf)
{
#ifdef DEBUG
	printf("\e[32mWrite AFTER text ...\e[0m\n");
#endif
	e->off = elf->text_program->p_offset + elf->text_program->p_filesz;
	elf->old_entry = (elf->text_program->p_vaddr + elf->text_program->p_filesz - elf->header->e_entry) * (-1);
	elf->text_offset = (elf->text_program->p_vaddr + elf->text_program->p_filesz - elf->text_section->sh_addr) * (-1);
	elf->header->e_entry = elf->text_program->p_vaddr + elf->text_program->p_filesz;

	elf->text_program->p_memsz += e->woody_total_size;
	elf->text_program->p_filesz += e->woody_total_size;
	elf->text_program->p_flags = PF_R | PF_W | PF_X;

	/* Had this line if you want to disassemble the injection portion with debuggers */
	// elf->text_section->sh_size += woody64_size;
}
void			modification_before_text_64(t_env *e, t_elf64 *elf)
{
#ifdef DEBUG
	printf("\e[34mWrite BEFORE text ...\e[0m\n");
#endif
	/* 
	** If we get here, it means we already ADD padding and write infection before the first section of the .text segment.
	** All we have to do is chack if we have space between the entry point and the start of the segment.
	** If not we ADD padding again.
	*/
	if (elf->header->e_entry - elf->text_program->p_vaddr < e->woody_total_size)
		return (modification_add_padding_64(e, elf));
	/*
	** Otherwise, we can write our code before
	*/
	/* Offset from where we will write our code */
	e->off = elf->header->e_entry - e->woody_total_size;
	/* Old entry point */
	elf->old_entry = e->woody_total_size;
	/* '.text' section offset from woody */
	elf->text_offset = elf->text_section->sh_addr - e->off;
	/* New entry point */
	elf->header->e_entry = e->off;

	elf->text_program->p_flags = PF_R | PF_W | PF_X;
}

static char		is_greater_than(Elf64_Addr value, Elf64_Phdr *prog)
{
	return (value >= prog->p_vaddr);
}

static char		d_tag_is_ptr(Elf64_Sxword tag)
{
	return ((tag >= DT_PLTGOT && tag <= DT_RELA)
		|| tag == DT_INIT || tag == DT_FINI || tag == DT_REL || tag == DT_DEBUG
		|| tag == DT_JMPREL || tag == DT_INIT_ARRAY || tag == DT_FINI_ARRAY
		|| tag == DT_PREINIT_ARRAY
		|| (tag >= DT_CONFIG && tag <= DT_SYMINFO)
		|| tag == DT_VERDEF || tag == DT_VERNEED);
}

void			modification_add_padding_64(t_env *e, t_elf64 *elf)
{
#ifdef DEBUG
	printf("\e[33mWrite ADD padding ...\e[0m\n");
#endif
	/* Compute the padding to add from segment alignement */
	while (e->padding < e->woody_total_size)
		e->padding += elf->text_program->p_align;
	/* Change Program Header offset */
	for (size_t i = 0; i < elf->header->e_phnum; i++)
	{
		if (elf->program[i].p_offset >= elf->text_program->p_offset + elf->text_program->p_filesz)
		{
			/* if (program is non-PIE) */
			// if (elf->text_program->p_vaddr + elf->text_program->p_memsz >= elf->program[i].p_vaddr)
			// {
			// 	ft_fatal("new Segment size too large. Risk of rewriting other Segment(s). Abort.", e);
			// }
			elf->program[i].p_offset += e->padding;
			elf->program[i].p_vaddr += e->padding;
			elf->program[i].p_paddr += e->padding;
		}
	}
	/* Change symbol, dynamic and relocation offsets */
	for (size_t i = 0; i < elf->header->e_shnum; i++)
	{
		/* SYMBOL */
		if (elf->section[i].sh_type == SHT_SYMTAB || elf->section[i].sh_type == SHT_DYNSYM)
		{
			size_t symbol_addr;
			Elf64_Sym *symbol;

			symbol_addr = (size_t)e->file + elf->section[i].sh_offset;
			symbol = (Elf64_Sym *)symbol_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++)
			{
				if (is_greater_than(symbol->st_value, elf->text_program))
				{
					symbol->st_value += e->padding;
				}
				symbol_addr += elf->section[i].sh_entsize;
				symbol = (Elf64_Sym *)symbol_addr;
			}
		}
		/* DYNAMIC */
		if (elf->section[i].sh_type == SHT_DYNAMIC)
		{
			size_t dynamic_addr;
			Elf64_Dyn *dynamic;

			dynamic_addr = (size_t)e->file + elf->section[i].sh_offset;
			dynamic = (Elf64_Dyn *)dynamic_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++)
			{
				if (is_greater_than(dynamic->d_un.d_val, elf->text_program) && d_tag_is_ptr(dynamic->d_tag))
				{
					dynamic->d_un.d_val += e->padding;
				}
				dynamic_addr += elf->section[i].sh_entsize;
				dynamic = (Elf64_Dyn *)dynamic_addr;
			}
		}
		/* RELOCATION */
		if (elf->section[i].sh_type == SHT_REL)
		{
			size_t rel_addr;
			Elf64_Rel *rel;

			rel_addr = (size_t)e->file + elf->section[i].sh_offset;
			rel = (Elf64_Rel *)rel_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++)
			{
				if (is_greater_than(rel->r_offset, elf->text_program))
				{
					rel->r_offset += e->padding;
				}
				rel_addr += elf->section[i].sh_entsize;
				rel = (Elf64_Rel *)rel_addr;
			}
		}
		if (elf->section[i].sh_type == SHT_RELA)
		{
			size_t rela_addr;
			Elf64_Rela *rela;

			rela_addr = (size_t)e->file + elf->section[i].sh_offset;
			rela = (Elf64_Rela *)rela_addr;
			for (size_t j = 0; j < elf->section[i].sh_size / elf->section[i].sh_entsize; j++)
			{
				if (is_greater_than(rela->r_offset, elf->text_program))
				{
					rela->r_offset += e->padding;
				}
				if (is_greater_than(rela->r_addend, elf->text_program))
				{
					rela->r_addend += e->padding;
				}
				rela_addr += elf->section[i].sh_entsize;
				rela = (Elf64_Rela *)rela_addr;
			}
		}
	}
	/* Change offsets in the section .got.plt */
	if (elf->header->e_shstrndx == SHN_UNDEF)
		ft_fatal("String table not reachable. Abort.", e);
	char *string_table = (char *)e->file + (elf->section + elf->header->e_shstrndx)->sh_offset;
	for (size_t i = 0; i < elf->header->e_shnum; i++)
	{
		Elf64_Shdr *shdr = (Elf64_Shdr *)(e->file + elf->header->e_shoff) + i;
		char *name = string_table + shdr->sh_name;

		if (ft_strcmp(name, ".got.plt") == 0)
		{
			void *ptr = (u_char *)e->file + shdr->sh_offset;
			for (size_t j = 0; j < shdr->sh_size; j += sizeof(unsigned long int))
			{
				if (is_greater_than(*(unsigned long int *)(ptr + j), elf->text_program))
				{
					*(unsigned long int *)(ptr + j) += e->padding;
				}
			}
			break;
		}
	}
	/* Change Section Header offest */
	for (size_t i = 0; i < elf->header->e_shnum; i++)
	{
		if (elf->section[i].sh_offset >= elf->text_program->p_offset)
		{
			elf->section[i].sh_offset += e->padding;
			if (elf->section[i].sh_addr >= elf->text_program->p_vaddr)
				elf->section[i].sh_addr += e->padding;
		}
	}
	elf->header->e_shoff += e->padding;
	elf->text_program->p_memsz += e->padding;
	elf->text_program->p_filesz += e->padding;
	elf->text_program->p_flags = PF_R | PF_W | PF_X;

	/* Woody offset from file (entry point) */
	Elf64_Addr woody_off = elf->text_program->p_vaddr + e->padding - e->woody_total_size;
	/* Offset from where we will stop to copy the initial program and start to inject our code,
	*  Here we have to take in count the padding */
	e->off = elf->text_program->p_offset;
	/* Old entry point offset calculated from woody */
	elf->old_entry = elf->header->e_entry + e->padding - woody_off;
	/* '.text' section offset calculated from woody */
	elf->text_offset = elf->text_section->sh_addr - woody_off;
	/* New entry point */
	elf->header->e_entry = woody_off;
}
