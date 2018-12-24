

namespace salgo {

namespace _internal {
namespace sequence_hash {

    template<int NUM_HASHES>
    struct Args {
        static constexpr int Num_Hashes = NUM_HASHES;
    };

    template<class ARGS>
    class Sequence_Hash {
        std::array<unsigned long long, ARGS::Num_Hashes> _hashes{};
    };

    template<class ARGS>
    struct With_Builder : Sequence_Hash {
        FORWARDING_CONSTRUCTOR(With_Builder, Sequence_Hash) {}

        template<int NEW_NUM_HASHES>
        using NUM_HASHES = With_Builder< Args< NEW_NUM_HASHES > >;
    }
}
}






using Sequence_Hash = _internal::sequence_hash::With_Builder< _internal::sequence_hash::Args<
    1 // num hashes
> >;



} // namespace salgo

