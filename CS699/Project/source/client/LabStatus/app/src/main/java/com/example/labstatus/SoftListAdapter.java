package com.example.labstatus;

import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;


/** @brief List adapter for software list

 @author Area51
 @date Nov 2019
 */

public class SoftListAdapter extends RecyclerView.Adapter<SoftListAdapter.MyViewHolder> {
    private ArrayList<String> mDataset;
    /**
    * Provide a reference to the views for each data item
    * Complex data items may need more than one view per item, and
    * you provide access to all the views for a data item in a view holder
     * @param filterdNames filetered list
    */
    public void filterList(ArrayList<String> filterdNames) {
        this.mDataset = filterdNames;
        notifyDataSetChanged();
    }

    /**
     *Each data item is just a string in this case
     *@see androidx.recyclerview.widget.RecyclerView.ViewHolder
     */
    public static class MyViewHolder extends RecyclerView.ViewHolder {

        public TextView textView;
        public MyViewHolder(TextView v) {
            super(v);
            textView = v;
        }
    }

    /**
     * Provide a suitable constructor (depends on the kind of dataset)
     * @param myDataset ArrayList Data Set
    */
     public SoftListAdapter(ArrayList<String> myDataset) {
        mDataset = myDataset;
    }
/**
   *  Create new views (invoked by the layout manager)
   */
    @Override
    public SoftListAdapter.MyViewHolder onCreateViewHolder(ViewGroup parent,
                                                     int viewType) {

        TextView v = (TextView) LayoutInflater.from(parent.getContext())
                .inflate(R.layout.list_item, parent, false);

        MyViewHolder vh = new MyViewHolder(v);
        return vh;
    }


    /**
     *  Replace the contents of a view (invoked by the layout manager)
     * @param holder replace the contents of the view with that element
     * @param position get element from your dataset at this position
     */
     @Override
    public void onBindViewHolder(MyViewHolder holder, int position) {

        holder.textView.setText(mDataset.get(position));

    }

    /**
     * Return the size of your dataset (invoked by the layout manager)
     * and assigning it to the list with notifydatasetchanged method
     * @return size of dataset
     */
         @Override
    public int getItemCount() {
        return mDataset.size();
    }


}