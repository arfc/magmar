#include "ElmIntegTotFissNtsPostprocessor.h"

registerMooseObject("MoltresApp", ElmIntegTotFissNtsPostprocessor);

InputParameters
ElmIntegTotFissNtsPostprocessor::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar(
      "group_fluxes",
      "The group fluxes. MUST be arranged by decreasing energy/increasing group number.");
  params.addRequiredParam<int>("num_groups", "The number of energy groups.");
  return params;
}

ElmIntegTotFissNtsPostprocessor::ElmIntegTotFissNtsPostprocessor(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    // MooseVariableInterface(this, false),
    _num_groups(getParam<int>("num_groups")),
    _nsf(getMaterialProperty<std::vector<Real>>("nsf")),
    _vars(getCoupledMooseVars())
{
  addMooseVariableDependency(_vars);
  int n = coupledComponents("group_fluxes");
  if (!(n == _num_groups))
    mooseError("The number of coupled variables doesn't match the number of groups.");

  _group_fluxes.resize(n);
  for (unsigned int i = 0; i < _group_fluxes.size(); ++i)
  {
    _group_fluxes[i] = &coupledValue("group_fluxes", i);
  }
}

Real
ElmIntegTotFissNtsPostprocessor::computeQpIntegral()
{
  Real sum = 0;
  for (int i = 0; i < _num_groups; ++i)
    sum += _nsf[_qp][i] * (*_group_fluxes[i])[_qp];

  return sum;
}
