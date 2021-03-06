/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/06/11 04:51:50 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/25 22:32:46 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include "libft.h"
#include "main.h"

static void		map_file(const char *prog, t_env *e)
{
	char		buf[1024];
	int			ret;
	int			i;

	if ((e->fd = open(prog, O_RDONLY)) == -1)
		ft_fatal(NULL, e);
	if ((e->file_size = lseek(e->fd, 1, SEEK_END)) == -1)
		ft_fatal(NULL, e);
	e->file = mmap(NULL, e->file_size, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (e->file == MAP_FAILED)
		ft_fatal(NULL, e);
	if ((lseek(e->fd, 0, SEEK_SET)) == -1)
		ft_fatal(NULL, e);
	i = 0;
	while ((ret = read(e->fd, buf, sizeof(buf))) > 0)
	{
		ft_memcpy(e->file + i, buf, ret);
		i += ret;
	}
	if (ret)
		ft_fatal(NULL, e);
	if (close(e->fd) == -1)
		perror("close()");
	e->fd = 0;
}

int				main(int ac, char **av)
{
	t_env		e;

	ft_memset(&e, 0, sizeof(e));
	e.file = MAP_FAILED;
	e.progname = ft_strrchr(av[0], '/');
	e.progname = (e.progname == NULL) ? av[0] : e.progname + 1;
	if (ac < 2 || ac > 3) {
		ft_printf("Usage: %s _program_ [banner]\n", e.progname);
		ft_printf("\t_program_\tBinary to infect.\n");
		ft_printf("\tbanner\t\tBanner to print when infected binary runs.\n");
		return (1);
	}
	ft_strncpy(e.banner, (av[2]) ? av[2] : "....WOODY....", 255);
	map_file(av[1], &e);
#ifdef __linux__
	check_elf_info(&e);
	generate_new_key(e.key);
	get_encryption_func(&e, av[0]);
	if (((Elf64_Ehdr *)e.file)->e_ident[EI_CLASS] == ELFCLASS32)
		pack_elf_32(&e);
	else if (((Elf64_Ehdr *)e.file)->e_ident[EI_CLASS] == ELFCLASS64)
		pack_elf_64(&e);
#elif __APPLE__
	check_macho_info(&e);
	generate_new_key(e.key);
	pack_macho64(&e);
#endif
	if (munmap(e.file, e.file_size) == -1)
		ft_fatal(NULL, &e);
	return (0);
}
