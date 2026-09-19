#include <pthread.h>
#include <system_error>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <exception>
#include <stdexcept>
#include <vector>

struct Task
{
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
void *taskAdapter(void *data)
{
  auto *task = static_cast< Task * >(data);

  task->result = calc(
      task->radius,
      task->tests,
      task->seed);

  return nullptr;
}
double area(double r, std::size_t threads, std::size_t tests)
{
  if (r <= 0.0 || threads == 0 || tests == 0)
  {
    throw std::invalid_argument("arguments must be positive");
  }

  if (threads > tests)
  {
    threads = tests;
  }

  std::vector< pthread_t > threadHandles(threads);
  std::vector< Task > tasks(threads);

  const std::size_t testsPerThread = tests / threads;
  const std::size_t remainder = tests % threads;
  const std::size_t baseSeed = std::random_device{}();

  std::size_t createdThreads = 0;

  for (std::size_t i = 0; i < threads; ++i)
  {
    const std::size_t threadTests =
        testsPerThread + (i < remainder ? 1 : 0);

    tasks[i] = Task{
        r,
        threadTests,
        baseSeed + i};

    const int error = pthread_create(
        &threadHandles[i],
        nullptr,
        taskAdapter,
        &tasks[i]);

    if (error != 0)
    {
      for (std::size_t j = 0; j < createdThreads; ++j)
      {
        pthread_join(threadHandles[j], nullptr);
      }

      throw std::system_error(
          error,
          std::generic_category(),
          "thread creation failed");
    }

    ++createdThreads;
  }

  int joinError = 0;

  for (std::size_t i = 0; i < createdThreads; ++i)
  {
    const int error = pthread_join(threadHandles[i], nullptr);

    if (error != 0 && joinError == 0)
    {
      joinError = error;
    }
  }

  if (joinError != 0)
  {
    throw std::system_error(
        joinError,
        std::generic_category(),
        "thread join failed");
  }

  std::size_t totalHits = 0;

  for (const Task &task : tasks)
  {
    totalHits += task.result;
  }

  return 4.0 * r * r *
         static_cast< double >(totalHits) /
         static_cast< double >(tests);
}

int main()
{
  constexpr std::size_t threadCount = 8;

  double radius = 0.0;
  std::size_t tests = 0;

  std::cin >> radius >> tests;

  try
  {
    if (!std::cin)
    {
      throw std::invalid_argument("failed to read input");
    }

    const double estimatedArea = area(
        radius,
        threadCount,
        tests);

    std::cout << std::fixed
              << std::setprecision(6)
              << estimatedArea
              << '\n';
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
