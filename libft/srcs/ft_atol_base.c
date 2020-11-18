/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atol_base.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/14 12:57:56 by gbourgeo          #+#    #+#             */
/*   Updated: 2020/11/14 13:04:10 by gbourgeo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

long		ft_atol_base(const char *str, const char *base)
{
	long	i;
	long	res;
	long	isneg;
	int		base_len;

	i = 0;
	res = 0;
	isneg = 1;
	base_len = ft_strlen(base);
	while (str[i] == ' ' || str[i] == '\n' || str[i] == '\t' || str[i] == '\v'
			|| str[i] == '\f' || str[i] == '\r')
		i++;
	if (str[i] == '-')
	{
		isneg = -1;
		i++;
	}
	else if (str[i] == '+')
		i++;
	while (str[i] && ft_strchr(base, str[i]))
	{
		res = res * base_len;
		res = res + (ft_strchr(base, str[i++]) - base);
	}
	return (res * isneg);
}
