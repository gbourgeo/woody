# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gbourgeo <gbourgeo@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2016/06/11 04:39:05 by gbourgeo          #+#    #+#              #
#    Updated: 2020/12/25 10:21:18 by gbourgeo         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= woody_woodpacker
ARCH	=
ifeq ($(32BIT), 1)
NAME	= woody_woodpacker32
ARCH	= -m32
endif

SRC_D	= srcs/
SRC		= main.c		\
		ft_fatal.c		\
		key_generator.c	\

HDR_DIR	= includes/

OBJ_DIR	= obj/
OBJ		= $(addprefix $(OBJ_DIR), $(SRC:.c=.o))
OBJ_S	= $(addprefix $(OBJ_DIR), $(SRC_S:.s=.o))

UNAME_S	:= $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S), Linux)
SRC_DIR		= $(SRC_D)elf/
SRC			+= check_elf_info.c #pack_elf.c

SRC_DIR_64	= $(SRC_DIR)64/
SRC			+= pack_elf_64.c	\
			get_elf_info_64.c	\
			modifications_64.c	\
			write_new_file_64.c	\

SRC_S		= woody64.s encrypt64.s

SRC_DIR_32	= $(SRC_DIR)32/
SRC			+= pack_elf_32.c	\
			get_elf_info_32.c	\
			modifications_32.c	\
			write_new_file_32.c	\

SRC_S		+= woody32.s encrypt32.s

ASMFLAG		= -f elf64
ifeq ($(32BIT), 1)
ASMFLAG		= -f elf32
endif
endif

ifeq ($(UNAME_S), Darwin)
SRC_DIR		= $(SRC_D)macho/
SRC			+= check_macho_info.c pack_macho64.c ft_swap_bytes.c
SRC_S		= woody64.s encrypt64.s
ASMFLAG		= -f macho64
endif

LIB_DIR	= libft/
LIB_HDR	= $(LIB_DIR)includes

ASM		= nasm
CC		= gcc $(ARCH)
CFLAGS	= -Wall -Werror -Wextra
ifeq ($(DEBUG), 1)
CFLAGS	+= -DDEBUG
endif
INCLUDE	= -I$(HDR_DIR) -I$(LIB_HDR)
LIBS	= -L$(LIB_DIR) -lft

all: lib $(NAME)

$(NAME): $(OBJ) $(OBJ_S)
	$(CC) -o $@ $^ $(LIBS)

$(OBJ_DIR)%.o: $(SRC_D)%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)
$(OBJ_DIR)%.o: $(SRC_DIR_64)%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)
$(OBJ_DIR)%.o: $(SRC_DIR_32)%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)

$(OBJ_DIR)%.o: $(SRC_DIR_64)%.s
	$(ASM) $(ASMFLAG) -o $@ $<
$(OBJ_DIR)%.o: $(SRC_DIR_32)%.s
	$(ASM) $(ASMFLAG) -o $@ $<

.PHONY: lib clean fclean re

lib:
	@make ARCH=$(ARCH) -sC $(LIB_DIR)
	@mkdir -p $(OBJ_DIR)

clean:
	@make -sC $(LIB_DIR) clean
	/bin/rm -rf $(OBJ_DIR)

fclean: clean
	@make -sC $(LIB_DIR) fclean
	/bin/rm -f $(NAME)
	/bin/rm -f $(NAME)32
	/bin/rm -f woody
	/bin/rm -f elf32
	/bin/rm -f elf64
	/bin/rm -f macho64

re: fclean all

elf:
	$(CC) -m64 -o elf64 Ressources/sample.c
	$(CC) -m32 -o elf32 Ressources/sample.c

macho:
	$(CC) -m64 -o macho64 Ressources/sample.c
	$(CC) -m32 -o macho32 Ressources/sample.c
