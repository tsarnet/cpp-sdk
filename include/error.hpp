#pragma once

#include <expected>
#include <system_error>

namespace tsar
{
    /// <summary>
    /// The error codes for the TSAR API.
    /// </summary>
    enum class error_code_t
    {
        /// <summary>
        /// Failed to get the user's HWID.
        /// </summary>
        failed_to_get_hwid_t,

        /// <summary>
        /// An unexpected error occurred.
        /// </summary>
        unexpected_error_t
    };

    /// <summary>
    /// The error category for the TSAR API.
    /// </summary>
    class error_category : public std::error_category
    {
       public:
        /// <summary>
        /// Gets the singleton instance of the error category.
        /// </summary>
        static const error_category& get() noexcept;

        /// <summary>
        /// Gets the name of the error category.
        /// </summary>
        const char* name() const noexcept override;

        /// <summary>
        /// Gets the message for the specified error code.
        /// </summary>
        std::string message( int ev ) const override;
    };

    /// <summary>
    /// Represents an error in the TSAR API.
    /// </summary>
    class error : public std::system_error
    {
       public:
        /// <summary>
        /// Creates a new error with the specified error code.
        /// </summary>
        /// <param name="code">The code.</param>
        explicit error( std::error_code code ) noexcept;

        /// <summary>
        /// Creates a new error with the specified error code value.
        /// </summary>
        /// <param name="code">The error code value.</param>
        explicit error( error_code_t code ) noexcept;
    };

}  // namespace tsar