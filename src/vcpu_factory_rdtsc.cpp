//
// Bareflank Hypervisor Examples
//
// Copyright (C) 2015 Assured Information Security, Inc.
// Author: Rian Quinn        <quinnr@ainfosec.com>
// Author: Brendan Kerrigan  <kerriganb@ainfosec.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include <bfbitmanip.h>
#include <vcpu/vcpu_factory.h>

#include <vcpu/vcpu_intel_x64.h>
#include <vmcs/vmcs_intel_x64.h>
#include <exit_handler/exit_handler_intel_x64.h>

class vmcs_rdtsc : public vmcs_intel_x64
{
public:

    vmcs_rdtsc() = default;
    ~vmcs_rdtsc() = default;

    void
    write_fields(gsl::not_null<vmcs_intel_x64_state *> host_state,
                 gsl::not_null<vmcs_intel_x64_state *> guest_state) override
    {
        vmcs_intel_x64::write_fields(host_state, guest_state);
        intel_x64::vmcs::primary_processor_based_vm_execution_controls::rdtsc_exiting::enable();
    }
};

class exit_handler_rdtsc : public exit_handler_intel_x64
{
public:

    exit_handler_rdtsc() = default;
    ~exit_handler_rdtsc() = default;

    void handle_exit(intel_x64::vmcs::value_type reason) override
    {
        using namespace intel_x64::vmcs::exit_reason;

        // NOTE:
        //
        // For completeness, CR4.TSD, CPL and CR0.PE should all be checked, otherwise
        // the execution of this instruction might bypass these checks by hardware.
        // For more information, please see the pseudo code for these instructions in
        // the Intel SDM.

        if (reason == basic_exit_reason::rdtsc) {
            auto ret = x64::read_tsc::get();
            m_state_save->rax = set_bits(m_state_save->rax, 0x00000000FFFFFFFF, ret >> 0);
            m_state_save->rdx = set_bits(m_state_save->rdx, 0x00000000FFFFFFFF, ret >> 32);

            advance_and_resume();
        }

        if (reason == basic_exit_reason::rdtscp) {
            auto ret = x64::read_tscp::get();
            m_state_save->rax = set_bits(m_state_save->rax, 0x00000000FFFFFFFF, ret >> 0);
            m_state_save->rdx = set_bits(m_state_save->rdx, 0x00000000FFFFFFFF, ret >> 32);
            m_state_save->rcx = set_bits(m_state_save->rcx, 0x00000000FFFFFFFF, x64::msrs::ia32_tsc_aux::get());

            advance_and_resume();
        }

        exit_handler_intel_x64::handle_exit(reason);
    }
};

/// Custom VCPU.
///
/// This tells Bareflank to use our exit handler instead of the one
/// that Bareflank provides by default.
///
std::unique_ptr<vcpu>
vcpu_factory::make_vcpu(vcpuid::type vcpuid, user_data *data)
{
    bfignored(data);

    return std::make_unique<vcpu_intel_x64>(
               vcpuid,
               nullptr,
               std::make_unique<vmcs_rdtsc>(),
               std::make_unique<exit_handler_rdtsc>(),
               nullptr,
               nullptr);
}
