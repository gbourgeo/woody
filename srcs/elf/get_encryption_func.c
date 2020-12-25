/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_encryption_func.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/25 22:02:30 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/25 22:11:16 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

void		woody32_encrypt(u_char *data, size_t len, const uint32_t *key);
void		woody64_encrypt(u_char *data, size_t len, const uint32_t *key);

void		get_encryption_func(t_env *e, char *prog)
{
	char	buf[sizeof(Elf64_Ehdr)];
	int		ret;

	if ((e->fd = open(prog, O_RDONLY)) == -1)
		ft_fatal(NULL, e);
	if ((ret = read(e->fd, buf, sizeof(buf))) < 0)
		ft_fatal(NULL, e);
	if (((Elf64_Ehdr *)buf)->e_ident[EI_CLASS] == ELFCLASS32)
		e->encrypt = woody32_encrypt;
	else if (((Elf64_Ehdr *)buf)->e_ident[EI_CLASS] == ELFCLASS64)
		e->encrypt = woody64_encrypt;
	else
		ft_fatal("Program is not in 32 or 64 bit...", e);
	close(e->fd);
	e->fd = 0;
}