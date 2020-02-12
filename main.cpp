#include "BPHelper.hpp"

using namespace std;

int main() {
    BPHelper bpHelper = BPHelper();
    bpHelper.requestActionThread();
    bpHelper.Receive();
}

