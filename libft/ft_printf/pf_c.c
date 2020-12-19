/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pf_c.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/08/16 04:14:59 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/11/18 20:07:35 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

void		pf_c(t_dt *data)
{
	int		ui[2];

	ui[0] = 1;
	ui[1] = va_arg(data->ap, int);
	if (!data->flag.minus)
	{
		while (data->flag.min_width > 1 && data->flag.min_width--)
			write_char(data, (data->flag.zero) ? '0' : ' ');
	}
	if ((*data->tail == 'C' || data->flag.len_modifier) && ui[1] > 127)
		write_wchar(data, (wchar_t *)&ui[1], ui);
	else
		write_char(data, ui[1]);
	if (data->flag.minus)
	{
		while (data->flag.min_width > 1 && data->flag.min_width--)
			write_char(data, ' ');
	}
}
