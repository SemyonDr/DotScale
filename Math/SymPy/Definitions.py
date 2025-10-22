


class GT_Definitions:

    # -----------
    # PRECISION
    # DEFINITIONS
    # -----------

    val_splines_in_unit: int = 64
    val_aprx_coef_bit_depth: int = 60

    area_splines_in_unit : int = 128
    area_aprx_coef_bit_depth : int = 60
    area_bit_depth : int = 63
    area_num_scales : int = 11

    # -----------
    # FILE NAMES
    # -----------

    @staticmethod
    def get_value_splines_file_name(splines_in_unit : int = None, fractional_precision : int = None) -> str:
        if splines_in_unit is None:
            splines_in_unit = GT_Definitions.val_splines_in_unit
        if fractional_precision is None:
            fractional_precision = GT_Definitions.val_aprx_coef_bit_depth
        return f"OutputData/value_approx_coef_{splines_in_unit}spiu_{fractional_precision}bit.data"

    @staticmethod
    def get_area_filename(scale : int, area_bit_depth : int = None):
        if area_bit_depth is None:
            area_bit_depth = GT_Definitions.area_bit_depth
        return f"OutputData/areas_{area_bit_depth}bit_scale_{scale}.data"

    @staticmethod
    def get_area_splines_file_name(splines_in_unit : int = None, fractional_precision : int = None) -> str:
        if splines_in_unit is None:
            splines_in_unit = GT_Definitions.area_splines_in_unit
        if fractional_precision is None:
            fractional_precision = GT_Definitions.area_aprx_coef_bit_depth
        return f"OutputData/area_approx_coef_{splines_in_unit}spiu_{fractional_precision}bit.data"


