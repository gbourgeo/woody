/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_new_file_64.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/14 00:18:44 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/14 00:47:05 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"
#include "libft.h"

extern uint32_t	woody64_size;
void			woody64_func(void);
void			woody64_encrypt(u_char *data, size_t len, const uint32_t *key);

void			write_new_file_64(t_env *e, t_elf64 *elf)
{
	int			banner_len;

	e->fd = open(OUTPUT_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 00755);
	if (e->fd == -1)
		ft_fatal(NULL, e);
	banner_len = ft_strlen(e->banner) + 1;
	woody64_encrypt((u_char *)(e->file + elf->text_section->sh_offset), elf->text_section->sh_size, e->key);
	write(e->fd, (char *)e->file, e->off);
	while (e->padding-- > e->woody_total_size)
		write(e->fd, "\0", 1);
	write(e->fd, &woody64_func, woody64_size);
	write(e->fd, e->key, sizeof(e->key));
	write(e->fd, &elf->text_offset, sizeof(elf->text_offset));
	write(e->fd, &elf->text_size, sizeof(elf->text_size));
	write(e->fd, &elf->old_entry, sizeof(elf->old_entry));
	write(e->fd, &banner_len, sizeof(banner_len));
	if (e->banner && *e->banner)
	{
		write(e->fd, e->banner, banner_len - 1);
		write(e->fd, "\n", 1);
	}
	while (e->modulo--)
		write(e->fd, "\0", 1);
	// e->off += e->woody_total_size;
	write(e->fd, (char *)e->file + e->off, e->file_size - e->off - 1);
	close(e->fd);
	e->fd = 0;
}
