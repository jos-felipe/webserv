# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: asanni <asanni@student.42sp.org.br>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/24 12:00:00 by josfelip          #+#    #+#              #
#    Updated: 2025/07/12 17:10:09 by asanni           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g
DEPFLAGS = -MMD -MP
DBGFLAGS = -g3 -D LOG_LEVEL=LOG_DEBUG

# Directories
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Binaries for debugging purposes
ifdef WITH_DEBUG
  NAME = debugme
  CXXFLAGS = $(DBGFLAGS)
  OBJ_DIR = obj_debug
endif

# Source files
SRC = $(SRC_DIR)/main.cpp \
      $(wildcard $(SRC_DIR)/config/*.cpp) \
      $(wildcard $(SRC_DIR)/http/*.cpp) \
      $(wildcard $(SRC_DIR)/socket/*.cpp) \
      $(wildcard $(SRC_DIR)/server/*.cpp) \
      $(wildcard $(SRC_DIR)/cgi/*.cpp) \
      $(wildcard $(SRC_DIR)/utils/*.cpp)

# Object files
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Dependency files for efficient rebuilding
DEP = $(OBJ:.o=.d)

# Include paths
INC_FLAGS = -I$(INC_DIR)

# Colors for terminal output
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
RESET = \033[0m

# Rules
all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(BLUE)Linking $(NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)$(NAME) successfully created!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling: $<$(RESET)"
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INC_FLAGS) -c $< -o $@

clean:
	@echo "$(BLUE)Removing object files...$(RESET)"
	@rm -rf $(OBJ_DIR) $(OBJ_DIR)_debug
	@echo "$(GREEN)Object files removed!$(RESET)"

fclean: clean
	@echo "$(BLUE)Removing $(NAME) and debugme...$(RESET)"
	@rm -f $(NAME)
	@rm -f debugme
	@echo "$(GREEN)$(NAME) and debugme removed!$(RESET)"

re: fclean all

debug:
	@make WITH_DEBUG=TRUE --no-print-directory

# Include dependency files
-include $(DEP)

.PHONY: all clean fclean re debug
