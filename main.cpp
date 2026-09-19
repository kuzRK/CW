#include <pthread.h>
#include <system_error>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>

struct Task {
    double radius;
    std::size_t tests;
    std::size_t seed;
    std::size_t result = 0;
};

bool isInside(double x, double y, double r)
{
  return (x - r) * (x - r) + (y - r) * (y - r) <= r * r;
}

std::size_t calc(double r, std::size_t tests, std::size_t seed)
{
  std::mt19937_64 generator(seed);
  std::uniform_real_distribution< double > coordinate(0.0, 2.0 * r);

  std::size_t inside = 0;

  for (std::size_t i = 0; i < tests; ++i)
  {
    const double x = coordinate(generator);
    const double y = coordinate(generator);

    if (isInside(x, y, r))
    {
      ++inside;
    }
  }

  return inside;
}
void* taskAdapter(void* data) {
    auto* task = static_cast< Task* >(data);

    task->result = calc(
        task->radius,
        task->tests,
        task->seed
    );

    return nullptr;
}

int main()
{
  double radius = 0.0;
  std::size_t tests = 0;

  std::cin >> radius >> tests;

  try
  {
    if (!std::cin)
    {
      throw std::invalid_argument("failed to read input");
    }

    if (radius <= 0.0 || tests == 0)
    {
      throw std::invalid_argument("arguments must be positive");
    }

    const std::size_t seed = std::random_device{}();
    const std::size_t inside = calc(radius, tests, seed);

    const double estimatedArea = 4.0 * radius * radius * static_cast< double >(inside) / static_cast< double >(tests);

    std::cout << std::fixed << std::setprecision(6) << estimatedArea << '\n';
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
