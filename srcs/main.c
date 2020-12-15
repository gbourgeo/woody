/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/06/11 04:51:50 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/12/14 00:35:57 by gbourgeo         ###   ########.fr       */
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

// extern uint32_t	woody64_size;
// extern uint32_t	woody32_size;
// void			woody64_encrypt(u_char *data, size_t len, const uint32_t *key);
// void			woody32_encrypt(u_char *data, size_t len, const uint32_t *key);
// void			woody64_func(void);
// void			woody32_func(void);

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
		ft_printf("\tbanner\tThe banner to print once the infected binary is infected.\n");
		return (1);
	}
	ft_strncpy(e.banner, (av[2]) ? av[2] : "....WOODY....", 255);
	e.banner_len = ft_strlen(e.banner) + 1;
	map_file(av[1], &e);
#ifdef __linux__
	check_elf_info(&e);
	generate_new_key(e.key);
	if (((Elf64_Ehdr *)e.file)->e_ident[EI_CLASS] == ELFCLASS32)
		// pack_elf(&e, , woody32_size, woody32_encrypt, woody32_func);
		pack_elf_32(&e);
	else if (((Elf64_Ehdr *)e.file)->e_ident[EI_CLASS] == ELFCLASS64)
		// pack_elf(&e, , woody64_size, woody64_encrypt, woody64_func);
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
