#include <iostream>

#include "tsar.hpp"

// You should have gotten these values after creating your app
// You can find them in your app's configuration settings
constexpr auto app_id = "00000000-0000-0000-0000-000000000000";
constexpr auto client_key = "MFk...";

// Basic function to format TSAR errors
static void error( const std::string_view title, const tsar::error& err ) noexcept
{
    std::cerr << "[AUTH] " << title << ": " << err.what() << std::endl;
}

int main()
{
  // This will create a new client & perform a hash check on your binary
  const auto client = tsar::client::create(app_id, client_key);

  if (!client)
  {
      error("Failed to create client", client.error());
      return 1;
  }

  std::println(std::cout, "[AUTH] Attempting to authenticate client...");

  // Check if user is authorized. By default the user's browser opens when auth fails. Passing `false` as an initial argument disables that.
  auto user = client->authenticate();

  // If they aren't authorized, continue to check until they've authenticated themselves in their browser.
  while (!user)
  {
      // Only continue the loop if the error type is "Unauthorized".
      if (user.error() != tsar::error_code_t::unauthorized_t)
      {
          error("Failed to authenticate", user.error());
          return 1;
      }

      std::this_thread::sleep_for(std::chrono::seconds(3)); // Keep a delay of at least 3 seconds to prevent rate-limiting.

      // Make sure to use false for any authenticate() function that's inside a loop, or else the browser will keep opening nonstop.
      user = client->authenticate(false);
  }

  // At this point the user is authenticated
  std::println(std::cout, "[AUTH] Successfully authenticated.");
  std::println(std::cout, "[AUTH] Welcome, {}.", user->name.value_or(user->id));

  // Start a heartbeat loop to continue checking if the user is authorized (we recommend running this in a background thread)
  //
  // **MAKE SURE THE LOOP RUNS ONLY ONCE EVERY 10 - 30 SECONDS**
  // Otherwise, your users might get rate-limited.
  //
  // Using a heartbeat thread will allow you to delete user sessions and have them be kicked off of your software live.
  // Additionally, if their subscription expires they will also be kicked during the heartbeat check.
  tsar::result_t< void > status;

  while (status = user->heartbeat())
  {
      std::println(std::cout, "[AUTH] Heartbeat success.");
      std::this_thread::sleep_for( std::chrono::seconds(20));
  }

  error("Heartbeat failed", status.error());
  return 1;
}
