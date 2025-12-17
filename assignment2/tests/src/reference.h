#pragma once
#include "../../maze/src/your_code_here_maze.h"
#include "../../amusement_park/src/your_code_here_amusement_park.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/matrix_decompose.hpp>
DISABLE_WARNINGS_POP()
#include <numeric>
#include <span>
#include <vector>

glm::mat3 referenceComputeMazeTransformation(std::span<const MovementCommand> moves);

TopSpinMatrices referenceComputeTopSpinTransformations(const TopSpinState& topSpinState);
