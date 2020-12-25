/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_new_file_32.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/14 00:18:44 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/25 22:24:27 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"
#include "libft.h"

extern uint32_t	woody32_size;
void			woody32_func(void);

#include<stdio.h>
void			write_new_file_32(t_env *e, t_elf32 *elf)
{
	uint32_t	banner_len;

	/* Open new file to inject our code */
	e->fd = open(OUTPUT_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 00755);
	if (e->fd == -1)
		ft_fatal(NULL, e);
	banner_len = ft_strlen(e->banner) + 1;
	/* Encrypt the .text section */
	e->encrypt((u_char *)(e->file + elf->text_section->sh_offset - e->padding), elf->text_section->sh_size, e->key);
	/* Write the same bytes until padding or injection */
	write(e->fd, (char *)e->file, e->off);
	if (e->padding)
		while (e->padding-- > e->woody_total_size)
			write(e->fd, "\0", 1);
	else
		e->off += e->woody_total_size;
	/* Write injection */
	write(e->fd, &woody32_func, woody32_size);
	write(e->fd, e->key, sizeof(e->key));
	write(e->fd, &elf->text_offset, sizeof((t_elf32){}.text_offset));
	write(e->fd, &elf->text_size, sizeof((t_elf32){}.text_size));
	write(e->fd, &elf->old_entry, sizeof((t_elf32){}.old_entry));
	write(e->fd, &banner_len, sizeof(banner_len));
	if (e->banner && *e->banner)
	{
		write(e->fd, e->banner, banner_len - 1);
		write(e->fd, "\n", 1);
	}
	/* Clean */
	while (e->modulo--)
		write(e->fd, "\0", 1);
	/* Write the rest of the file */
	write(e->fd, (char *)e->file + e->off, e->file_size - e->off - 1);
	close(e->fd);
	e->fd = 0;
}
