//
// Copyright (C) 2019 Assured Information Security, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <bfvmm/vcpu/vcpu_factory.h>
#include <bfvmm/hve/arch/intel_x64/vcpu.h>

using namespace ::intel_x64::vmcs;

namespace example
{
namespace intel_x64
{

class vcpu : public bfvmm::intel_x64::vcpu
{
public:

    vcpu(vcpuid::type id) :
        bfvmm::intel_x64::vcpu{id}
    {
        vmcs_n::primary_processor_based_vm_execution_controls::rdtsc_exiting::enable();

        this->add_handler(
            exit_reason::basic_exit_reason::rdtsc,
            ::handler_delegate_t::create<vcpu, &vcpu::handle_rdtsc>(this)
        );

        this->add_handler(
            exit_reason::basic_exit_reason::rdtscp,
            ::handler_delegate_t::create<vcpu, &vcpu::handle_rdtscp>(this)
        );
    }

    bool
    handle_rdtsc(gsl::not_null<vcpu_t *> vcpu)
    {
        bfignored(vcpu);

        // NOTE:
        //
        // For completeness, CR4.TSD, CPL and CR0.PE should all be checked, otherwise
        // the execution of this instruction might bypass these checks by hardware.
        // For more information, please see the pseudo code for these instructions in
        // the Intel SDM.

        auto ret = x64::read_tsc::get();
        this->set_rax((ret >> 0) & 0x00000000FFFFFFFF);
        this->set_rdx((ret >> 32) & 0x00000000FFFFFFFF);

        return this->advance();
    }

    bool
    handle_rdtscp(gsl::not_null<vcpu_t *> vcpu)
    {
        bfignored(vcpu);

        // NOTE:
        //
        // For completeness, CR4.TSD, CPL and CR0.PE should all be checked, otherwise
        // the execution of this instruction might bypass these checks by hardware.
        // For more information, please see the pseudo code for these instructions in
        // the Intel SDM.

        auto ret = x64::read_tscp::get();
        this->set_rax((ret >> 0) & 0x00000000FFFFFFFF);
        this->set_rdx((ret >> 32) & 0x00000000FFFFFFFF);
        this->set_rcx(x64::msrs::ia32_tsc_aux::get() & 0x00000000FFFFFFFF);

        return this->advance();
    }
};

}
}

namespace bfvmm
{

WEAK_SYM std::unique_ptr<vcpu>
vcpu_factory::make(vcpuid::type vcpuid, bfobject *obj)
{
    bfignored(obj);
    return std::make_unique<example::intel_x64::vcpu>(vcpuid);
}

}
