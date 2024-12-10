MAKEFLAGS += --no-print-directory
NAME = ircserv
COMPILER = c++
FLAGS = -Wall -Wextra -Werror --std=c++98
RM = rm -rf
SRCDIR = srcs
OBJDIR = objs
SRCS = $(wildcard $(SRCDIR)/*.cpp) main.cpp
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.cpp=.o)))

all: $(NAME)
		@echo "\033[5;32mCompilation Successful\033[0m"

$(NAME): $(OBJDIR) $(OBJS)
		@$(COMPILER) $(FLAGS) -o $(NAME) $(OBJS)

$(OBJDIR):
		@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
		@echo "Now compiling \033[0;36m$<\033[0m"
		@$(COMPILER) $(FLAGS) -c $< -o $@

$(OBJDIR)/main.o: main.cpp
		@echo "Now compiling \033[0;36m$<\033[0m"
		@$(COMPILER) $(FLAGS) -c $< -o $@

clean:
		@echo "\033[0;31mRemoving /objs folder\033[0m"
		@$(RM) $(OBJDIR)

fclean: clean
		@echo "\033[0;31mRemoving binaries\033[0m"
		@$(RM) $(NAME)

re: fclean
		@$(MAKE)