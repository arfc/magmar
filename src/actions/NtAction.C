#include "NtAction.h"
#include "Factory.h"
#include "Parser.h"
#include "Conversion.h"
#include "FEProblem.h"

template<>
InputParameters validParams<NtAction>()
{
  InputParameters params = validParams<AddVariableAction>();
  params.addRequiredParam<int>("num_precursor_groups", "specifies the total number of precursors to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<VariableName>("temperature", "Name of temperature variable");
  params.addRequiredParam<int>("num_groups", "The total number of energy groups.");
  params.addRequiredParam<bool>("use_exp_form", "Whether concentrations should be in an expotential/logarithmic format.");
  params.addParam<bool>("jac_test", false, "Whether we're testing the Jacobian and should use some random initial conditions for the precursors.");
  params.addParam<bool>("use_source_stabilization", false, "Whether to use source stabilization.");
  params.addParam<Real>("offset", "The value by which to offset the logarithmic stabilization.");
  params.addParam<std::vector<BoundaryName> >("vacuum_boundaries", "The boundaries on which to apply vacuum boundaries.");
  return params;
}

NtAction::NtAction(const InputParameters & params) :
    AddVariableAction(params),
    _num_precursor_groups(getParam<int>("num_precursor_groups")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _num_groups(getParam<int>("num_groups"))
{
}

void
NtAction::act()
{
  std::vector<VariableName> all_var_names;
  for (int op = 1; op <= _num_groups; ++op)
    all_var_names.push_back(_var_name_base + Moose::stringify(op));

  for (int op = 1; op <= _num_groups; ++op)
  {
    //
    // Create variable names
    //

    std::string var_name = _var_name_base + Moose::stringify(op);

    if (_current_task == "add_variable")
      addVariable(var_name);

    if (_current_task == "add_kernel")
    {

      // Set up time derivatives

      {
        InputParameters params = _factory.getValidParams("NtTimeDerivative");
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<int>("group_number") = op;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        if (isParamValid("use_exp_form"))
          params.set<bool>("use_exp_form") = getParam<bool>("use_exp_form");
        if (isParamValid("temperature"))
          params.set<std::vector<VariableName> >("temperature") = {getParam<VariableName>("temperature")};

        std::string kernel_name = "NtTimeDerivative_" + var_name;
        _problem->addKernel("NtTimeDerivative", kernel_name, params);
      }

      // Set up GroupDiffusion

      {
        InputParameters params = _factory.getValidParams("GroupDiffusion");
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<int>("group_number") = op;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        if (isParamValid("use_exp_form"))
          params.set<bool>("use_exp_form") = getParam<bool>("use_exp_form");
        if (isParamValid("temperature"))
          params.set<std::vector<VariableName> >("temperature") = {getParam<VariableName>("temperature")};

        std::string kernel_name = "GroupDiffusion_" + var_name;
        _problem->addKernel("GroupDiffusion", kernel_name, params);
      }

      // Set up SigmaR

      {
        InputParameters params = _factory.getValidParams("SigmaR");
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<int>("group_number") = op;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        if (isParamValid("use_exp_form"))
          params.set<bool>("use_exp_form") = getParam<bool>("use_exp_form");
        if (isParamValid("temperature"))
          params.set<std::vector<VariableName> >("temperature") = {getParam<VariableName>("temperature")};

        std::string kernel_name = "SigmaR_" + var_name;
        _problem->addKernel("SigmaR", kernel_name, params);
      }

      // Set up InScatter

      {
        InputParameters params = _factory.getValidParams("InScatter");
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<int>("group_number") = op;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        if (isParamValid("use_exp_form"))
          params.set<bool>("use_exp_form") = getParam<bool>("use_exp_form");
        if (isParamValid("temperature"))
          params.set<std::vector<VariableName> >("temperature") = {getParam<VariableName>("temperature")};
        params.set<int>("num_groups") = _num_groups;
        // params.set<std::vector<VariableName> >("group_fluxes") = getParam<std::vector<VariableName> >("group_fluxes");
        params.set<std::vector<VariableName> >("group_fluxes") = all_var_names;

        std::string kernel_name = "InScatter_" + var_name;
        _problem->addKernel("InScatter", kernel_name, params);
      }

      // Set up CoupledFissionKernel

      {
        InputParameters params = _factory.getValidParams("CoupledFissionKernel");
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<int>("group_number") = op;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        if (isParamValid("use_exp_form"))
          params.set<bool>("use_exp_form") = getParam<bool>("use_exp_form");
        if (isParamValid("temperature"))
          params.set<std::vector<VariableName> >("temperature") = {getParam<VariableName>("temperature")};
        params.set<int>("num_groups") = _num_groups;
        // params.set<std::vector<VariableName> >("group_fluxes") = getParam<std::vector<VariableName> >("group_fluxes");
        params.set<std::vector<VariableName> >("group_fluxes") = all_var_names;

        std::string kernel_name = "CoupledFissionKernel_" + var_name;
        _problem->addKernel("CoupledFissionKernel", kernel_name, params);
      }
    }

    if (_current_task == "add_bc")
    {
      if (isParamValid("vacuum_boundaries"))
      {

        // Set up vacuum boundary conditions

        InputParameters params = _factory.getValidParams("VacuumConcBC");
        params.set<std::vector<BoundaryName> >("boundary") = getParam<std::vector<BoundaryName> >("vacuum_boundaries");
        params.set<NonlinearVariableName>("variable") = var_name;
        if (isParamValid("use_exp_form"))
          params.set<bool>("use_exp_form") = getParam<bool>("use_exp_form");
        std::string bc_name = "VacuumConcBC_" + var_name;
        _problem->addBoundaryCondition("VacuumConcBC", bc_name, params);
      }
    }

    if (_current_task == "add_ic")
    {
      if (getParam<bool>("jac_test"))
      {
        InputParameters params = _factory.getValidParams("RandomIC");
        params.set<VariableName>("variable") = var_name;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        params.set<Real>("min") = 0;
        params.set<Real>("max") = 1;

        std::string ic_name = "RandomIC_" + var_name;
        _problem->addInitialCondition("RandomIC", ic_name, params);
      }

      else
      {
        InputParameters params = _factory.getValidParams("ConstantIC");
        params.set<VariableName>("variable") = var_name;
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");
        if (getParam<bool>("use_exp_form"))
          params.set<Real>("value") = 0;
        else
          params.set<Real>("value") = 1;

        std::string ic_name = "ConstantIC_" + var_name;
        _problem->addInitialCondition("ConstantIC", ic_name, params);
      }
    }


    if (getParam<bool>("use_exp_form"))
    {

      std::string aux_var_name = var_name + "_lin";

      // Set up nodal aux variables

      if (_current_task == "add_aux_variable")
      {
        std::set<SubdomainID> blocks = getSubdomainIDs();
        FEType fe_type(FIRST, LAGRANGE);
        if (blocks.empty())
          _problem->addAuxVariable(aux_var_name, fe_type);
        else
          _problem->addAuxVariable(aux_var_name, fe_type, &blocks);
      }

      // Set up aux kernels

      if (_current_task == "add_aux_kernel")
      {
        InputParameters params = _factory.getValidParams("Density");
        params.set<AuxVariableName>("variable") = aux_var_name;
        params.set<std::vector<VariableName> >("density_log") = {var_name};
        if (isParamValid("block"))
          params.set<std::vector<SubdomainName> >("block") = getParam<std::vector<SubdomainName> >("block");

        std::string aux_kernel_name = "Density_" + aux_var_name;
        _problem->addAuxKernel("Density", aux_kernel_name, params);
      }
    }
  }

  if (_current_task == "add_variable")
  {
    std::string temp_var = "temp";
    addVariable(temp_var);
  }
}