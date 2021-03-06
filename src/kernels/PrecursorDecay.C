#include "PrecursorDecay.h"

registerMooseObject("MoltresApp", PrecursorDecay);

InputParameters
PrecursorDecay::validParams()
{
  InputParameters params = Kernel::validParams();
  params += ScalarTransportBase::validParams();
  params.addRequiredParam<unsigned int>("precursor_group_number",
                                        "What precursor group this kernel is acting on.");
  params.addRequiredCoupledVar("temperature",
                               "The temperature used to interpolate material properties.");
  params.addParam<Real>("prec_scale", 1, "The amount by which to scale precursors.");
  return params;
}

PrecursorDecay::PrecursorDecay(const InputParameters & parameters)
  : Kernel(parameters),
    ScalarTransportBase(parameters),
    _decay_constant(getMaterialProperty<std::vector<Real>>("decay_constant")),
    _d_decay_constant_d_temp(getMaterialProperty<std::vector<Real>>("d_decay_constant_d_temp")),
    _precursor_group(getParam<unsigned int>("precursor_group_number") - 1),
    _temp_id(coupled("temperature")),
    _temp(coupledValue("temperature")),
    _prec_scale(getParam<Real>("prec_scale"))
{
}

Real
PrecursorDecay::computeQpResidual()
{
  return _test[_i][_qp] * _decay_constant[_qp][_precursor_group] * computeConcentration(_u, _qp) *
         _prec_scale;
}

Real
PrecursorDecay::computeQpJacobian()
{
  return _test[_i][_qp] * _decay_constant[_qp][_precursor_group] *
         computeConcentrationDerivative(_u, _phi, _j, _qp) * _prec_scale;
}

Real
PrecursorDecay::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _temp_id)
    return _test[_i][_qp] * _d_decay_constant_d_temp[_qp][_precursor_group] * _phi[_j][_qp] *
           computeConcentration(_u, _qp) * _prec_scale;

  else
    return 0;
}
