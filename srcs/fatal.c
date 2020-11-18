/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fatal.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/05/11 11:54:48 by root              #+#    #+#             */
/*   Updated: 2020/10/25 17:29:55 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "ft_fprintf.h"
#include "main.h"

/*
** PRINTS ERROR, FREE RESSOURCES AND EXIT.
** If first argument is NULL, it will call 'perror', otherwise it will print the string.
** Second argument is for freeing the ressources used.
*/
int			ft_fatal(char *str, t_env *e)
{
	ft_fprintf(stderr, "%s: ", e->progname);
	if (!str)
		perror(str);
	else
		ft_fprintf(stderr, "%s\n", str);
	if (e->fd > 0)
		close(e->fd);
	if (e->file != MAP_FAILED)
		munmap(e->file, e->file_size);
	remove(OUTPUT_FILENAME);
	exit(1);
}
