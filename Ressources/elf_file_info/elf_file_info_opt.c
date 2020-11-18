/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_file_info_opt.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/08 20:12:21 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/11/18 19:09:42 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "elf_file_info.h"

static void		check_error(char *value, t_opt *opt, size_t opt_size, t_env *e)
{
	size_t		i;

	if (e->error) {
		if (value)
			ft_printf("%s: %s: '%s'\n", e->prog, e->error, value);
		else
			ft_printf("%s: %s\n", e->prog, e->error);
		ft_printf("\nUsage:\n");
		ft_printf("\t%s", e->prog);
		i = 0;
		while (i < opt_size)
		{
			ft_printf(" [-%s]", opt[i].opt);
			i++;
		}
		ft_printf(" file\n");
		i = 0;
		while (i < opt_size)
		{
			ft_printf("\t\t-%s\t%s\n", opt[i].opt, opt[i].description);
			i++;
		}
		exit(1);
	}
}

static int	opt_h(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_g(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_c(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_y(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_d(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_x(char **av, int *i, t_env *e, t_opt *opt)
{
	size_t	len;
	t_sec	*ptr;

	e->option |= OPT_X_SECTION;
	if (!av[*i + 1])
	{
		e->error = "Missing parameter";
		return(1);
	}
	len = e->x_len;
	ptr = ft_memalloc((len + 1) * sizeof(*e->x_section));
	if (ptr == NULL)
	{
		e->error = "New allocation failed";
		return (1);
	}
	ft_memcpy(ptr, e->x_section, sizeof(*ptr) * len);
	ptr[len].name = av[*i + 1];
	ptr[len].bit |= opt->bit;
	free(e->x_section);
	e->x_section = ptr;
	e->x_len++;
	(*i)++;
	return (0);
}

static int	opt_LE(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_BE(char **av, int *i, t_env *e, t_opt *opt)
{
	e->option |= opt->bit;
	return (0);
}

static int	opt_H(char **av, int *i, t_env *e, t_opt *opt)
{
	e->error = "Help wanted";
	return (1);
}

void		parse_opt(char **av, t_env *e)
{
	t_opt	opt[] = {
		{ "h",  "Print file Header",                          OPT_HEADER,        opt_h },
		{ "g",  "Print file Segments",                        OPT_SEGMENT,       opt_g },
		{ "c",  "Print file Sections",                        OPT_SECTION,       opt_c },
		{ "y",  "Print file Symbol Table",                    OPT_SYMBOL,        opt_y },
		{ "d",  "Print file Dynamic Section",                 OPT_DYNAMIC,       opt_d },
		{ "xg", "Print hexa/text/long_int specified Section", OPT_XG_SECTION,    opt_x },
		{ "xd", "Print hexa/text/int      specified Section", OPT_XD_SECTION,    opt_x },
		{ "xw", "Print hexa/text/short    specified Section", OPT_XW_SECTION,    opt_x },
		{ "xx", "Print hexa/text/char     specified Section", OPT_XX_SECTION,    opt_x },
		{ "x",  "Print hexa/text          specified Section", OPT_X_SECTION,     opt_x },
		{ "LE", "Force output to be print in little endian",  OPT_LITTLE_ENDIAN, opt_LE },
		{ "BE", "Force output to be print in big endian",     OPT_BIG_ENDIAN,    opt_BE },
		{ "H",  "Print Help",                                 OPT_HELP,          opt_H },
	};
	int		i;
	int		j;
	char	*ptr;

	i = 1;
	if (av)
		while (av[i])
		{
			if (*av[i] == '-')
			{
				ptr = &av[i][1];
				while (*ptr && e->error == NULL)
				{
					j = 0;
					while (j < (int)(sizeof(opt) / sizeof(opt[0]))
					&& ft_strncmp(ptr, opt[j].opt, ft_strlen(opt[j].opt)))
						j++;
					if (j == (int)(sizeof(opt) / sizeof(opt[0])))
						e->error = "Illegal option";
					else if (opt[j].func(av, &i, e, opt + j) != 0)
						check_error(av[i], opt, sizeof(opt) / sizeof(opt[0]), e);
					ptr += ft_strlen(opt[j].opt);
				}
			}
			else if (e->file != NULL)
				e->error = "Too many arguments";
			else
				e->file = av[i];
			check_error(av[i], opt, sizeof(opt) / sizeof(opt[0]), e);
			i++;
		}
	if (e->file == NULL)
		e->error = "Missing file";
	if (e->option == 0)
		e->error = "No options specified";
	check_error(av[i], opt, sizeof(opt) / sizeof(opt[0]), e);
}
