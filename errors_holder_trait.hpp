#ifndef _ERRORS_HOLDER_TRAIT_HPP_
#define _ERRORS_HOLDER_TRAIT_HPP_

#include <vector>
#include <string>

namespace pl0::error {

class ErrorsHolderTrait {
public:
    ErrorsHolderTrait() = default;
    virtual ~ErrorsHolderTrait() = default;

    constexpr auto hadError() const -> bool {
        return !m_errors.empty();
    }

    constexpr auto errors() const -> const std::vector<std::string>& {
        return m_errors;
    }

protected:
    
    constexpr auto pushError(std::string error) -> void {
        m_errors.push_back(std::move(error));
    }

private:
    std::vector<std::string> m_errors;
};

}


#endif
